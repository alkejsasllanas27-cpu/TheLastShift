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

	DisableLegacyFlashlightComponents();

	bIsOn = bStartsOn;
	SpotLight->SetVisibility(bIsOn);
	FlashlightMesh->SetVisibility(bIsOn);

	TryBindInput();
	// NOTE: tick is intentionally left enabled (see TickComponent) -- it also
	// keeps the hand attachment self-healing for the lifetime of the component.
}

void UFlashlightComponent::DisableLegacyFlashlightComponents()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// An earlier prototype left a hand-placed flashlight mesh and spot light on the
	// Blueprint. They cannot be deleted (the Blueprint graph still references them,
	// and removing them fails to compile), and clearing them from the editor does not
	// survive into play. So they are silenced here, on the live instance, every time
	// play begins -- otherwise the player sees two extra flashlights floating in view.
	TArray<UStaticMeshComponent*> StaticMeshes;
	Owner->GetComponents<UStaticMeshComponent>(StaticMeshes);
	for (UStaticMeshComponent* Mesh : StaticMeshes)
	{
		if (Mesh && Mesh != FlashlightMesh)
		{
			Mesh->SetStaticMesh(nullptr);
			Mesh->SetVisibility(false);
			Mesh->SetHiddenInGame(true);
			Mesh->SetCastShadow(false);
		}
	}

	TArray<USpotLightComponent*> SpotLights;
	Owner->GetComponents<USpotLightComponent>(SpotLights);
	for (USpotLightComponent* Light : SpotLights)
	{
		if (Light && Light != SpotLight)
		{
			Light->SetIntensity(0.f);
			Light->SetVisibility(false);
			Light->SetHiddenInGame(true);
			Light->SetCastShadows(false);
			Light->SetAttenuationRadius(0.f);
			Light->bAffectsWorld = false;
			Light->MarkRenderStateDirty();
		}
	}
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

	// Only the parent/socket RELATIONSHIP is established here. The transform itself is
	// recomputed every tick from GetSocketLocation/Rotation (see UpdateAimedTransform)
	// rather than left to rigid attachment, because the holding pose is written into
	// the mesh's component-space bones AFTER animation: socket queries observe that
	// override, but a rigidly attached child does not, and would visibly lag behind
	// the posed arm.
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

	// Two naming conventions are supported: Epic's mannequin rig (upperarm_r / hand_r)
	// and the Mixamo rig (RightArm / RightHand), so a downloaded character can be
	// dropped in without the holding pose silently doing nothing.
	static const TArray<FName> EpicChain = {
		"upperarm_r", "upperarm_twist_01_r", "upperarm_twist_02_r",
		"lowerarm_r", "lowerarm_twist_01_r", "lowerarm_twist_02_r", "hand_r",
		"index_metacarpal_r", "index_01_r", "index_02_r", "index_03_r",
		"middle_metacarpal_r", "middle_01_r", "middle_02_r", "middle_03_r",
		"ring_metacarpal_r", "ring_01_r", "ring_02_r", "ring_03_r",
		"pinky_metacarpal_r", "pinky_01_r", "pinky_02_r", "pinky_03_r",
		"thumb_01_r", "thumb_02_r", "thumb_03_r"
	};
	static const TArray<FName> MixamoChain = {
		"RightArm", "RightForeArm", "RightHand",
		"RightHandThumb1", "RightHandThumb2", "RightHandThumb3", "RightHandThumb4",
		"RightHandIndex1", "RightHandIndex2", "RightHandIndex3", "RightHandIndex4",
		"RightHandMiddle1", "RightHandMiddle2", "RightHandMiddle3", "RightHandMiddle4",
		"RightHandRing1", "RightHandRing2", "RightHandRing3", "RightHandRing4",
		"RightHandPinky1", "RightHandPinky2", "RightHandPinky3", "RightHandPinky4"
	};

	FName PivotBone("upperarm_r");
	const TArray<FName>* Chain = &EpicChain;
	int32 PivotIndex = Mesh->GetBoneIndex(PivotBone);
	if (PivotIndex == INDEX_NONE)
	{
		PivotBone = FName("RightArm");
		PivotIndex = Mesh->GetBoneIndex(PivotBone);
		Chain = &MixamoChain;
	}
	if (PivotIndex == INDEX_NONE)
	{
		return;
	}
	const TArray<FName>& ChainBones = *Chain;

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
	if (bIsOn)
	{
		BatteryCharge = FMath::Max(0.f, BatteryCharge - DeltaTime / FMath::Max(30.f, BatteryRuntimeSeconds));
		if (BatteryCharge <= 0.f) SetFlashlightOn(false);
	}

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
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return;
	}

	UCameraComponent* Camera = Pawn->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		return;
	}

	USkeletalMeshComponent* HandMesh = Cast<USkeletalMeshComponent>(GetAttachParent());
	if (!HandMesh)
	{
		return;
	}

	// The MESH takes BOTH its location and rotation from the hand socket, so it is
	// oriented exactly the way the fingers are curled and reads as genuinely gripped.
	// Doing this per tick (rather than via rigid attachment) is deliberate: socket
	// queries see the holding-pose bone override, a rigid child would not.
	const FQuat SocketQuat = HandMesh->GetSocketQuaternion(HandSocketName);
	const FQuat MeshRotation = SocketQuat * GripOffsetRotation.Quaternion();
	FVector FinalLocation = HandMesh->GetSocketLocation(HandSocketName) + MeshRotation.RotateVector(GripOffsetLocation);
	if (!FMath::IsNearlyZero(GripDepthOffset))
	{
		FinalLocation -= MeshRotation.GetForwardVector() * GripDepthOffset;
	}
	SetWorldLocationAndRotation(FinalLocation, MeshRotation);

	// The BEAM is steered independently, straight from the camera. The arm mesh hangs
	// off the body rather than the camera, so a beam inheriting the hand's rotation
	// would never aim true when looking up or down. Splitting mesh from beam is what
	// lets the flashlight look held AND land exactly on the crosshair at once.
	SpotLight->SetWorldRotation(Camera->GetComponentQuat());
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
	bIsOn = bNewOn && BatteryCharge > 0.f;
	SpotLight->SetVisibility(bIsOn);
	FlashlightMesh->SetVisibility(bIsOn);
}

bool UFlashlightComponent::ReplaceBattery()
{
	if (BatteryCharge >= 0.98f) return false;
	BatteryCharge = 1.f;
	return true;
}
