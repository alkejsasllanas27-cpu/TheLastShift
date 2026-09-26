#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShiftMonster.generated.h"

class UStaticMeshComponent;
class USpotLightComponent;
class UAudioComponent;
class USoundBase;
class UAnimSequence;

UCLASS()
class THELASTSHIFT_API AShiftMonster : public ACharacter
{
 GENERATED_BODY()
public:
 AShiftMonster();
 virtual void OnConstruction(const FTransform& Transform) override;
 virtual void BeginPlay() override;
 virtual void Tick(float DeltaSeconds) override;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Creature;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UAudioComponent> Voice;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror") TObjectPtr<USoundBase> Scream;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror") TObjectPtr<USoundBase> Growl;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation") TObjectPtr<UAnimSequence> WalkAnimation;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation") TObjectPtr<UAnimSequence> IdleAnimation;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror") float DetectionDistance=2400.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror") float ChaseSpeed=235.f;
 /** Reach of the grab: once the player is this close the jumpscare starts and the run is over. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror") float CatchDistance=165.f;
 /** How long the creature is held in the player's face before the screen fades and the game restarts. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror") float JumpscareSeconds=1.7f;
 /** Distance from the eyes to the creature's head during the grab. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror") float JumpscareFaceDistance=155.f;
 /** Where the lunge starts from: the creature is thrown in from here to JumpscareFaceDistance. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror") float JumpscareLungeFrom=260.f;
 /** Blood-red backdrop that fills the screen behind the creature during the grab. */
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror") TObjectPtr<UStaticMeshComponent> ScareBackdrop;
 /** Hard light thrown on the creature during the grab, so the whole face reads at once. */
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror") TObjectPtr<USpotLightComponent> ScareLight;
 /** Peak camera shake of the grab, in degrees. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror") float JumpscareShake=3.2f;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror") bool bChasing=false;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror") bool bCaughtPlayer=false;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror") int32 ScreamCount=0;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Horror") int32 AttackCount=0;
private:
 bool bWalkingAnimation=false;
 FVector VisualOrigin=FVector::ZeroVector;
 FVector LastSeen=FVector::ZeroVector;
 float Memory=0.f, ScreamCooldown=0.f, FreezeTime=0.f, GrowlCooldown=5.f, Age=0.f;
 float CatchTime=0.f;
 bool bFadeStarted=false, bRestartRequested=false;
 bool CanSee(APawn* Target) const;
 FVector Steer(const FVector& Desired, APawn* Target) const;
 void Roar();
 /** Freezes the player, throws the creature into view and starts the countdown to the restart. */
 void CatchPlayer(APawn* Target);
 /** Holds the creature in the player's face, fades to black and reloads the level from the start. */
 void JumpscareTick(APawn* Target, float Dt);
};
