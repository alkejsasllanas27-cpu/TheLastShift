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
enum class EShiftObjectKind : uint8 { Battery, Drawer, Cabinet, HidingCabinet, Note, Switch };

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
 void UpdateObjective(const FString& Objective);
private:
 UPROPERTY() TObjectPtr<UTextBlock> ObjectiveText;
 UPROPERTY() TObjectPtr<UTextBlock> BatteryText;
 UPROPERTY() TObjectPtr<class UProgressBar> BatteryBar;
 UPROPERTY() TObjectPtr<UTextBlock> PromptText;
 UPROPERTY() TObjectPtr<UTextBlock> StatusText;
 UPROPERTY() TObjectPtr<UTextBlock> NoteBody;
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
private:
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
