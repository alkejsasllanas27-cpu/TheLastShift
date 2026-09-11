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

	/**
	 * Extra offset from the hand socket's location, for fine-tuning grip -- measured
	 * in AIM space (X = further along where the player is looking, Y = right, Z = up),
	 * so it's intuitive to tune regardless of how the hand bone itself is oriented.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Attachment")
	FVector GripOffsetLocation = FVector::ZeroVector;

	/** Small constant rotation applied on top of the camera's aim rotation, for fine-tuning grip angle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Attachment")
	FRotator GripOffsetRotation = FRotator::ZeroRotator;

	/**
	 * Shifts the flashlight backward (in cm) along its OWN current forward axis
	 * after attaching, so more of its body sits inside a closed fist instead of
	 * only the tail touching the grip point. Intuitive/unambiguous, unlike
	 * GripOffsetLocation (which is in the socket's own, not-obvious local axes).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Attachment")
	float GripDepthOffset = 0.f;

	/**
	 * Rigidly rotates the whole right arm (from the shoulder down through every
	 * finger bone) by this amount around the shoulder joint, ONLY while the
	 * flashlight is on -- raising it into a natural "holding something up" pose.
	 * Applied as one rigid rotation of the entire sub-chain (not per-joint), so the
	 * hand's existing grip/finger pose is preserved exactly, just re-oriented; this
	 * cannot tear the arm apart the way rotating individual bones independently can.
	 * Positive pitch raises the arm forward/up; positive yaw swings it inward.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|Pose")
	FRotator HoldingPoseArmRotation = FRotator(35.f, 10.f, 0.f);

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

	/**
	 * Every tick: positions the flashlight at the hand socket's current (animated)
	 * location, but drives its ROTATION directly from the camera. This rig's hand
	 * bone only turns with yaw, not pitch, so using the socket's own rotation would
	 * mean the beam never follows looking up/down -- this guarantees it always does.
	 */
	void UpdateAimedTransform();

	/** Bound to the arm mesh's OnBoneTransformsFinalized so our pose override applies AFTER animation, not before (otherwise it would just get overwritten). */
	void OnArmBoneTransformsFinalized();

	TWeakObjectPtr<USkeletalMeshComponent> BoundArmMesh;
	FDelegateHandle ArmBoneTransformsFinalizedHandle;
};
