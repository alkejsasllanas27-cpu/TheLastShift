#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FlashlightComponent.generated.h"

class UStaticMeshComponent;
class USpotLightComponent;
class USkeletalMeshComponent;
class UInputAction;
class UInputMappingContext;

/**
 * Self-contained handheld flashlight: owns its own mesh + spotlight as a rigid
 * parent/child pair, so the beam always points exactly where the mesh points.
 * Attach this to a camera (or any SceneComponent) on a Pawn; it binds its own
 * Enhanced Input toggle action, so no Blueprint event-graph wiring is required.
 * Designed to be extended later (battery, flicker, sound, wear) without touching callers.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THELASTSHIFT_API UFlashlightComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UFlashlightComponent();

	/** The physical flashlight mesh, visible in first person. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
	TObjectPtr<UStaticMeshComponent> FlashlightMesh;

	/** The beam. Attached as a child of FlashlightMesh so it inherits its exact rotation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
	TObjectPtr<USpotLightComponent> SpotLight;

	/** Input Action used to toggle the flashlight on/off (defaults to IA_Flashlight). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Input")
	TObjectPtr<UInputAction> ToggleAction;

	/** Mapping context that contains ToggleAction's key binding (defaults to IMC_Flashlight). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	/** Priority used when adding InputMappingContext to the local player's input subsystem. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Input")
	int32 InputMappingPriority = 1;

	/** Whether the flashlight starts switched on when play begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight")
	bool bStartsOn = false;

	/**
	 * Optional: a skeletal mesh (e.g. the first-person arms) to visually hold the
	 * flashlight in-hand. If set, the component re-attaches to HandSocketName on
	 * this mesh at BeginPlay, so it renders gripped in the hand instead of floating
	 * in front of the camera. Leave empty to keep it parented wherever it was placed
	 * in the Blueprint (e.g. directly on the camera, for guaranteed 1:1 aim tracking).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Attachment")
	TObjectPtr<USkeletalMeshComponent> HandMeshComponent;

	/** Socket on HandMeshComponent to attach to (e.g. a hand-grip socket). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Attachment")
	FName HandSocketName = "HandGrip_R";

	/** Extra local adjustment applied after attaching to the hand socket, for fine-tuning grip. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Attachment")
	FVector GripOffsetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Attachment")
	FRotator GripOffsetRotation = FRotator::ZeroRotator;

	/** Flips the flashlight between on and off. Safe to call from Blueprint or other C++ systems. */
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void ToggleFlashlight();

	/** Explicitly sets the on/off state. */
	UFUNCTION(BlueprintCallable, Category = "Flashlight")
	void SetFlashlightOn(bool bNewOn);

	UFUNCTION(BlueprintPure, Category = "Flashlight")
	bool IsFlashlightOn() const { return bIsOn; }

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool bIsOn = false;
	bool bInputBound = false;

	/** Lazily binds Enhanced Input once the owning Pawn has a valid PlayerController input component. */
	void TryBindInput();

	/**
	 * Finds the hand mesh (explicit HandMeshComponent, or a sibling skeletal mesh
	 * component on the same actor that has HandSocketName) and attaches to it.
	 * Called from OnRegister (so it also looks correct in the editor / Blueprint
	 * preview, not only during Play) and BeginPlay (guaranteed-safe fallback for
	 * actual gameplay, in case sibling components weren't registered yet in OnRegister).
	 */
	void TryAttachToHand(bool bRequireTargetRegistered);
};
