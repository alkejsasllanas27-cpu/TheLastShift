#include "FlashlightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

UFlashlightComponent::UFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	FlashlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashlightMesh"));
	FlashlightMesh->SetupAttachment(this);
	FlashlightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlashlightMesh->SetCastShadow(true);
	FlashlightMesh->SetOnlyOwnerSee(false);

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

	AttachToComponent(ResolvedHandMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocketName);
	SetRelativeLocation(GripOffsetLocation);
	SetRelativeRotation(GripOffsetRotation);
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
}
