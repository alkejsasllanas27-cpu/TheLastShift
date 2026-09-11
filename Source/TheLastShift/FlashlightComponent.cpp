#include "FlashlightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/KismetSystemLibrary.h"

UFlashlightComponent::UFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	FlashlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashlightMesh"));
	FlashlightMesh->SetupAttachment(this);
	FlashlightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlashlightMesh->SetCastShadow(true);
	FlashlightMesh->SetOnlyOwnerSee(false);
	FlashlightMesh->SetVisibility(false);

	// Attached to the mesh (not to this component) so the beam is rigidly locked
	// to the mesh's lens end and always rotates exactly with it.
	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(FlashlightMesh);
	SpotLight->SetRelativeLocation(FVector(13.f, 0.f, 0.f));
	SpotLight->SetRelativeRotation(FRotator::ZeroRotator);

	SpotLight->Intensity = 3000.f;
	SpotLight->IntensityUnits = ELightUnits::Unitless;
	SpotLight->InnerConeAngle = 15.f;
	SpotLight->OuterConeAngle = 25.f;
	SpotLight->AttenuationRadius = 1500.f;
	SpotLight->SetLightColor(FLinearColor(FColor(214, 244, 255)));
	SpotLight->CastShadows = true;
	SpotLight->SourceRadius = 1.f;
	SpotLight->bUseInverseSquaredFalloff = true;
	SpotLight->Mobility = EComponentMobility::Movable;
	SpotLight->SetVisibility(false);
}

void UFlashlightComponent::OnRegister()
{
	Super::OnRegister();
	// Sibling components may not be registered yet this early (e.g. in the editor
	// preview, or during actor spawn) -- attaching to an unregistered component is
	// unsafe, so only attempt this opportunistically here; BeginPlay below always
	// retries unconditionally as the guaranteed-safe path for actual gameplay.
	TryAttachToHand(/*bRequireTargetRegistered=*/true);
}

void UFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();

	TryAttachToHand(/*bRequireTargetRegistered=*/false);

	bIsOn = bStartsOn;
	SpotLight->SetVisibility(bIsOn);
	FlashlightMesh->SetVisibility(bIsOn);

	TryBindInput();
	// NOTE: tick is intentionally left enabled (see TickComponent) -- it also
	// keeps the hand attachment self-healing for the lifetime of the component.
}

void UFlashlightComponent::TryAttachToHand(bool bRequireTargetRegistered)
{
	if (HandSocketName.IsNone())
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner || Owner->HasAnyFlags(RF_ArchetypeObject | RF_ClassDefaultObject))
	{
		// Skip template/CDO/preview contexts -- there is no real actor instance yet
		// to attach to, and attempting it triggers an engine ensure.
		return;
	}

	// Resolve the hand mesh on THIS actor instance by searching its OWN components.
	// We deliberately do NOT trust the HandMeshComponent property here: a reference
	// to another component set on the Blueprint template can end up pointing at a
	// template/archetype object rather than this actor's live instance, causing
	// AttachToComponent to silently fail. Only fall back to it if nothing on this
	// actor matches. Prefer an exact "FirstPersonMesh" name match first: the
	// character also has a third-person body mesh ("Mesh") that shares the same
	// skeleton/sockets, which a socket-only search could ambiguously match instead.
	USkeletalMeshComponent* ResolvedHandMesh = nullptr;
	{
		TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
		Owner->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);

		for (USkeletalMeshComponent* Candidate : SkeletalMeshComponents)
		{
			if (Candidate && Candidate->GetFName() == FName("FirstPersonMesh"))
			{
				ResolvedHandMesh = Candidate;
				break;
			}
		}
		if (!ResolvedHandMesh)
		{
			for (USkeletalMeshComponent* Candidate : SkeletalMeshComponents)
			{
				if (Candidate && Candidate->DoesSocketExist(HandSocketName))
				{
					ResolvedHandMesh = Candidate;
					break;
				}
			}
		}
	}
	if (!ResolvedHandMesh && HandMeshComponent && HandMeshComponent->GetOwner() == Owner)
	{
		ResolvedHandMesh = HandMeshComponent;
	}

	if (!ResolvedHandMesh || (bRequireTargetRegistered && !ResolvedHandMesh->IsRegistered()))
	{
		return;
	}

	// Already correctly attached -- nothing to do. Checking this every tick (rather
	// than a one-shot "did it once" flag) is what makes this self-healing: if
	// something external resets our parent later (e.g. a component re-register
	// triggered by World Partition streaming), the next tick notices and fixes it.
	if (GetAttachParent() == ResolvedHandMesh && GetAttachSocketName() == HandSocketName)
	{
		return;
	}

	// Only the parent/socket RELATIONSHIP is established here; the actual transform
	// is fully recomputed every tick in TickComponent (see UpdateAimedTransform),
	// because this rig's hand socket only turns with yaw, not pitch -- relying on
	// its rotation would mean the beam ignores looking up/down entirely.
	AttachToComponent(ResolvedHandMesh, FAttachmentTransformRules::KeepWorldTransform, HandSocketName);

	if (BoundArmMesh != ResolvedHandMesh)
	{
		if (BoundArmMesh.IsValid())
		{
			BoundArmMesh->UnregisterOnBoneTransformsFinalizedDelegate(ArmBoneTransformsFinalizedHandle);
			RemoveTickPrerequisiteComponent(BoundArmMesh.Get());
		}
		ArmBoneTransformsFinalizedHandle = ResolvedHandMesh->RegisterOnBoneTransformsFinalizedDelegate(
			FOnBoneTransformsFinalizedMultiCast::FDelegate::CreateUObject(this, &UFlashlightComponent::OnArmBoneTransformsFinalized));
		// Guarantees our tick (which reads the hand socket's position) always runs
		// AFTER the arm's animation update + pose override above, avoiding a
		// one-frame lag between the raised pose and where the flashlight sits.
		AddTickPrerequisiteComponent(ResolvedHandMesh);
		BoundArmMesh = ResolvedHandMesh;
	}
}

void UFlashlightComponent::OnArmBoneTransformsFinalized()
{
	if (!bIsOn || !BoundArmMesh.IsValid())
	{
		return;
	}
	USkeletalMeshComponent* Mesh = BoundArmMesh.Get();

	static const FName PivotBone("upperarm_r");
	static const TArray<FName> ChainBones = {
		"upperarm_r", "upperarm_twist_01_r", "upperarm_twist_02_r",
		"lowerarm_r", "lowerarm_twist_01_r", "lowerarm_twist_02_r", "hand_r",
		"index_metacarpal_r", "index_01_r", "index_02_r", "index_03_r",
		"middle_metacarpal_r", "middle_01_r", "middle_02_r", "middle_03_r",
		"ring_metacarpal_r", "ring_01_r", "ring_02_r", "ring_03_r",
		"pinky_metacarpal_r", "pinky_01_r", "pinky_02_r", "pinky_03_r",
		"thumb_01_r", "thumb_02_r", "thumb_03_r"
	};

	const int32 PivotIndex = Mesh->GetBoneIndex(PivotBone);
	if (PivotIndex == INDEX_NONE)
	{
		return;
	}

	TArray<FTransform>& ComponentSpaceTransforms = Mesh->GetEditableComponentSpaceTransforms();
	const FVector PivotLocation = ComponentSpaceTransforms[PivotIndex].GetLocation();
	const FQuat DeltaRotation = HoldingPoseArmRotation.Quaternion();

	// Resolve bone indices and snapshot original transforms first, then write them
	// all back rotated -- so writing an earlier bone in the chain can't affect what
	// we read for a later one.
	TArray<int32> BoneIndices;
	TArray<FTransform> OriginalTransforms;
	BoneIndices.Reserve(ChainBones.Num());
	OriginalTransforms.Reserve(ChainBones.Num());
	for (const FName& BoneName : ChainBones)
	{
		const int32 Index = Mesh->GetBoneIndex(BoneName);
		if (Index != INDEX_NONE)
		{
			BoneIndices.Add(Index);
			OriginalTransforms.Add(ComponentSpaceTransforms[Index]);
		}
	}

	for (int32 i = 0; i < BoneIndices.Num(); ++i)
	{
		const FTransform& Old = OriginalTransforms[i];
		const FVector NewLocation = PivotLocation + DeltaRotation.RotateVector(Old.GetLocation() - PivotLocation);
		const FQuat NewRotation = DeltaRotation * Old.GetRotation();
		ComponentSpaceTransforms[BoneIndices[i]] = FTransform(NewRotation, NewLocation, Old.GetScale3D());
	}
}

void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInputBound)
	{
		TryBindInput();
	}

	// Self-healing: level streaming (this project uses World Partition) can
	// unregister/re-register components shortly after spawn, which resets an
	// SCS-instanced component's attachment back to its Blueprint-authored default.
	// Re-checking every tick (cheap: just a parent/socket comparison, see below)
	// guarantees the flashlight stays gripped in the hand no matter when that happens.
	TryAttachToHand(/*bRequireTargetRegistered=*/false);

	UpdateAimedTransform();
}

void UFlashlightComponent::UpdateAimedTransform()
{
	USkeletalMeshComponent* HandMesh = Cast<USkeletalMeshComponent>(GetAttachParent());
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!HandMesh || !Pawn)
	{
		return;
	}

	UCameraComponent* Camera = Pawn->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		return;
	}

	// Position: follows the hand socket's actual (animated) location every frame,
	// so it visually moves and sways with the hand during walking/idling.
	// Rotation: driven directly from the camera, NOT the socket -- this rig's hand
	// bone only turns with yaw (confirmed by measurement), so using its rotation
	// would mean the beam never responds to looking up/down. Driving rotation from
	// the camera guarantees the beam always points exactly where the player aims.
	const FVector HandLocation = HandMesh->GetSocketLocation(HandSocketName);
	const FQuat AimRotation = GripOffsetRotation.Quaternion() * Camera->GetComponentQuat();

	FVector FinalLocation = HandLocation + AimRotation.RotateVector(GripOffsetLocation);
	if (!FMath::IsNearlyZero(GripDepthOffset))
	{
		FinalLocation -= AimRotation.GetForwardVector() * GripDepthOffset;
	}

	SetWorldLocationAndRotation(FinalLocation, AimRotation);
}

void UFlashlightComponent::TryBindInput()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn || !ToggleAction)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC)
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(Pawn->InputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	if (InputMappingContext)
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->AddMappingContext(InputMappingContext, InputMappingPriority);
			}
		}
	}

	EnhancedInput->BindAction(ToggleAction, ETriggerEvent::Started, this, &UFlashlightComponent::ToggleFlashlight);
	bInputBound = true;
}

void UFlashlightComponent::ToggleFlashlight()
{
	SetFlashlightOn(!bIsOn);
}

void UFlashlightComponent::SetFlashlightOn(bool bNewOn)
{
	bIsOn = bNewOn;
	SpotLight->SetVisibility(bIsOn);
	FlashlightMesh->SetVisibility(bIsOn);
}
