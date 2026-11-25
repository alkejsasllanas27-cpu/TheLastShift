#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "ShiftGameplay.generated.h"

class UStaticMeshComponent;
class UTextBlock;
class UBoxComponent;
class UAudioComponent;
class USoundBase;
class UFlashlightComponent;
class ACharacter;
class AShiftDirector;
class AShiftLadder;

UENUM(BlueprintType)
enum class EShiftObjectKind : uint8 { Battery, Drawer, Cabinet, HidingCabinet, Note, Switch, Flashlight };

/** One reusable interaction contract for authored objects. Legacy doors are adapted by the director. */
UCLASS()
class THELASTSHIFT_API AShiftInteractable : public AActor
{
 GENERATED_BODY()
public:
 AShiftInteractable();
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") EShiftObjectKind Kind=EShiftObjectKind::Drawer;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") FText NoteText;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") FText DisplayName;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance") TObjectPtr<UStaticMesh> DrawerBody;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance") TArray<TObjectPtr<UStaticMesh>> DrawerMeshes;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") TObjectPtr<AShiftInteractable> Container;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio") TObjectPtr<USoundBase> OpenSound;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio") TObjectPtr<USoundBase> CloseSound;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audio") TObjectPtr<USoundBase> UseSound;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Story") FName StoryEvent;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction") bool bOpen=false;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction") bool bOccupied=false;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction") float OpenAlpha=0;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USceneComponent> MovingRoot;
 UFUNCTION(BlueprintCallable, CallInEditor) void Rebuild();
 UFUNCTION(BlueprintCallable) void Interact(AShiftDirector* Director);
 UFUNCTION(BlueprintPure) FText GetPrompt() const;
 UFUNCTION(BlueprintPure) bool IsAvailable() const;
 void SetOpen(bool Value);
 virtual void Tick(float DeltaSeconds) override;
 virtual void OnConstruction(const FTransform& Transform) override;
private:
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
 UPROPERTY() TArray<TObjectPtr<class UPointLightComponent>> Lights;
 UStaticMeshComponent* Part(FName Name,FVector Location,FVector Size,USceneComponent* Parent,bool Metal=false);
 void ApplyPose();
};

UCLASS()
class THELASTSHIFT_API UShiftPromptWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 virtual void NativeOnInitialized() override;
 void Show(const FString& Prompt,const FString& Status,const FString& Note);
 void UpdateBattery(float Charge,int32 Spares,bool On);
 void SetBatteryVisible(bool Visible);
 /** Swaps the HUD for the death card the creature's grab ends on. */
 void ShowDeath();
 /** The carried-items panel, shown while the player holds it open. */
 void ShowInventory(const FString& Lines);
 void UpdateObjective(const FString& Objective);
private:
 UPROPERTY() TObjectPtr<UTextBlock> ObjectiveText;
 UPROPERTY() TObjectPtr<UTextBlock> BatteryText;
 UPROPERTY() TObjectPtr<class UProgressBar> BatteryBar;
 UPROPERTY() TObjectPtr<UTextBlock> PromptText;
 UPROPERTY() TObjectPtr<UTextBlock> StatusText;
 UPROPERTY() TObjectPtr<UTextBlock> NoteBody;
 UPROPERTY() TObjectPtr<UTextBlock> DeathText;
 UPROPERTY() TObjectPtr<UTextBlock> InventoryText;
};

UCLASS()
class THELASTSHIFT_API UShiftMenuWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 virtual void NativeOnInitialized() override;
 virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
 UPROPERTY() TObjectPtr<AShiftDirector> Director;
 UFUNCTION() void Resume();
 UFUNCTION() void Quality();
 UFUNCTION() void VSync();
 UFUNCTION() void WindowMode();
 UFUNCTION() void Resolution();
 UFUNCTION() void FrameLimit();
 UFUNCTION() void Quit();
 void Refresh();
private:
 UPROPERTY() TObjectPtr<UTextBlock> QualityText;
 UPROPERTY() TObjectPtr<UTextBlock> VSyncText;
 UPROPERTY() TObjectPtr<UTextBlock> ModeText;
 UPROPERTY() TObjectPtr<UTextBlock> ResolutionText;
 UPROPERTY() TObjectPtr<UTextBlock> FrameText;
};

/** Single player trace/input owner; no per-prop polling. */
UCLASS()
class THELASTSHIFT_API AShiftDirector : public AActor
{
 GENERATED_BODY()
public:
 AShiftDirector();
 virtual void Tick(float Dt) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival") int32 SpareBatteries=0;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival") bool bPlayerHidden=false;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction") TObjectPtr<AActor> FocusedActor;
 UFUNCTION(BlueprintCallable) void Interact();
 UFUNCTION(BlueprintCallable) void ReloadBattery();
 UFUNCTION(BlueprintCallable) void AddBattery();
 UFUNCTION(BlueprintCallable) void PickUpFlashlight();
 /** Called by the creature when its grab lands: the HUD becomes the death card until the reload. */
 UFUNCTION(BlueprintCallable) void PlayerCaught();
 /** Crouches or stands the player, the same path the crouch key drives. Standing needs headroom. */
 UFUNCTION(BlueprintCallable) void SetCrouched(bool bValue);
 UFUNCTION(BlueprintCallable) void ToggleInventory();
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival") bool bDead=false;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival") bool bCrouched=false;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival") bool bInventoryOpen=false;
 /** Shoulder width while crouched: narrow enough to slip through the gaps a standing player cannot. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival") float SqueezeRadius=16.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival") float CrouchedHalfHeight=52.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival") float CrouchSpeed=115.f;
 /** Story items the player is carrying, newest last. */
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival") TArray<FString> CarriedItems;
 UFUNCTION(BlueprintCallable) void EnterHiding(AShiftInteractable* Cabinet);
 UFUNCTION(BlueprintCallable) void ExitHiding();
 UFUNCTION(BlueprintCallable) void ReadNote(const FText& Text,FName StoryEvent);
 UFUNCTION(BlueprintCallable) void Notify(const FString& Text,float Seconds=3.f);
 UFUNCTION(BlueprintCallable) void TriggerStory(FName Event);
 UFUNCTION(BlueprintCallable) void ToggleMenu();
 UFUNCTION(BlueprintPure) UShiftMenuWidget* GetMenu() const { return Menu; }
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) bool bMenuOpen=false;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Story") int32 ObjectiveStage=0;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Traversal") TObjectPtr<AShiftLadder> ActiveLadder;
 /** Seconds a hinged door takes to swing. Matches the creak: after ~2.2 s it is only reverb fading out. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Doors") float DoorSwingSeconds=2.2f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Doors") TObjectPtr<USoundBase> DoorCreakSound;
 /** Played when a closing door reaches the frame. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Doors") TObjectPtr<USoundBase> DoorLatchSound;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Doors") float DoorVolume=.8f;
 /**
  * Exposure bias the flashlight was tuned at, inside the hospital and outside. Where an area runs a different
  * exposure (the interiors are darker), the beam is scaled so it still looks the way it was tuned.
  */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Lighting") float FlashlightInteriorReferenceBias=9.5f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Lighting") float FlashlightOutdoorReferenceBias=10.1f;
 /** Opens or closes a hinged door (an actor with bIsOpen and OpenAngle), eased and timed to its creak. Double doors go together. */
 UFUNCTION(BlueprintCallable) void SwingDoor(AActor* Door);
private:
 /**
  * The hospital doors' actor origin is the building's origin (every door of a wing shares it), so
  * turning the actor swings the leaf metres away. A door turns around its real hinge edge instead:
  * the level tags each door "Hinge.Min"/"Hinge.Max" (the edge away from its handle, or the outer edge
  * of a double door) and "DoorPair.<actor name>" for the other leaf of a double door.
  */
 struct FDoorHinge{FVector Pivot=FVector::ZeroVector;FVector ClosedLocation=FVector::ZeroVector;FQuat ClosedRotation=FQuat::Identity;FTransform ClosedLeaf;float Angle=0;bool bHingeAtMin=true;};
 /** From/To are angles around the hinge: 0 is closed, +-OpenAngle open. */
 struct FDoorSwing{TWeakObjectPtr<AActor> Door;float From=0,To=0,Elapsed=0,Duration=1;bool bClosing=false;};
 TArray<FDoorSwing> DoorSwings;
 /** Each hinged door's closed pose and hinge, worked out the first time it is used. */
 TMap<TWeakObjectPtr<AActor>,FDoorHinge> DoorHinges;
 FDoorHinge FindHinge(AActor* Door,UStaticMeshComponent* Leaf,float OpenAngle) const;
 /** Whether the leaf, turned to Angle, would run into a wall (the half by the hinge is ignored: it touches the frame anyway). */
 bool DoorPoseBlocked(AActor* Door,UStaticMeshComponent* Leaf,const FTransform& ClosedLeaf,const FVector& Pivot,bool bHingeAtMin,float Angle) const;
 void SetDoorAngle(AActor* Door,FDoorHinge& Hinge,float Angle);
 void SwingOne(AActor* Door,bool bPlaySound);
 AActor* DoorPartner(AActor* Door) const;
 void UpdateDoors(float Dt);
 /** Leaves that let the player pass while they move, with the pawn response to put back once the player is clear. */
 TMap<TWeakObjectPtr<UPrimitiveComponent>,TEnumAsByte<ECollisionResponse>> DoorsIgnoringPawn;
 /** The player input component whose old Blueprint E binding (the asset pack's door snap) has been removed. */
 TWeakObjectPtr<UObject> InteractBindingCleared;
 /** Rain around the player, told each frame whether there is a roof overhead. */
 UPROPERTY() TObjectPtr<class AShiftRain> Rain;
 /** The hospital's bounded exposure volumes, and the unbounded outdoor one. */
 TArray<TWeakObjectPtr<class APostProcessVolume>> InteriorVolumes;
 TWeakObjectPtr<class APostProcessVolume> OutdoorVolume;
 void UpdateWeatherAndExposure(float Dt);
 UPROPERTY() TObjectPtr<UShiftMenuWidget> Menu;
 UPROPERTY() TObjectPtr<ACharacter> Player;
 UPROPERTY() TObjectPtr<APlayerController> PC;
 UPROPERTY() TObjectPtr<UFlashlightComponent> Flashlight;
 UPROPERTY() TObjectPtr<UShiftPromptWidget> Widget;
 UPROPERTY() TObjectPtr<AShiftInteractable> HidingCabinet;
 UPROPERTY() TMap<TObjectPtr<AActor>,TObjectPtr<AActor>> DoorControllers;
 float TraceClock=0,StatusUntil=0;
 FString Status,Reading;
 FRotator HideFacing;
 uint8 SavedMovementMode=0;
 bool bSavedCollision=true,bWasLow=false;
 bool BindPlayer();
 void UpdateFocus();
 FString Prompt() const;
 /** Crouch narrows the capsule too, so tight gaps become passable; standing needs room above. */
 void UpdateCrouch();
 FString InventoryLines() const;
 float StandRadius=34.f,StandHalfHeight=96.f,StandSpeed=500.f;
 bool bCrouchLatched=false;
};

/** Starts the shared player interaction owner in gameplay worlds; no map rebuild. */
UCLASS()
class THELASTSHIFT_API UShiftGameplaySubsystem : public UWorldSubsystem
{
 GENERATED_BODY()
public:
 virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
 virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};

UCLASS()
class THELASTSHIFT_API AShiftAudioZone : public AActor
{
 GENERATED_BODY()
public:
 AShiftAudioZone();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UBoxComponent> Bounds;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UAudioComponent> Ambient;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UAudioComponent> Story;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Audio") TObjectPtr<USoundBase> AmbientSound;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Audio") TObjectPtr<USoundBase> StorySound;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Audio") TObjectPtr<USoundBase> TensionSound;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Audio") float FadeSeconds=3.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Audio") float AmbientVolume=.2f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Audio") FName StoryEvent;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Audio") bool bTriggerStoryOnEnter=false;
 UFUNCTION(BlueprintCallable) void PlayStory(bool bTension=false);
protected:
 virtual void BeginPlay() override;
 UFUNCTION() void Enter(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComponent,int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);
 UFUNCTION() void Leave(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComponent,int32 OtherBodyIndex);
private:
 bool bPlayed=false;
};
