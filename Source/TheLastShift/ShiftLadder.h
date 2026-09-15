#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShiftLadder.generated.h"

class ACharacter;
class APlayerController;
class UBoxComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

/** Upright ladder: local +X faces the climber, origin is the lower floor. Input belongs to ShiftDirector. */
UCLASS()
class THELASTSHIFT_API AShiftLadder : public AActor
{
 GENERATED_BODY()
public:
 AShiftLadder();
 virtual void OnConstruction(const FTransform& Transform) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;

 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ladder", meta=(ClampMin="120", Units="cm")) float Height=320.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ladder", meta=(ClampMin="35", Units="cm")) float Width=60.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ladder", meta=(ClampMin="30", Units="cm/s")) float ClimbSpeed=145.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ladder", meta=(ClampMin="40", Units="cm")) float StandOff=60.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ladder", meta=(ClampMin="40", Units="cm")) float EntryReach=170.f;
 /** Additional height above the landing needed to clear a balcony guardrail. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ladder", meta=(ClampMin="0", Units="cm")) float TopClearance=0.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ladder", meta=(ClampMin="10", Units="cm")) float MaxLandingAdjustment=80.f;
 /** Floor position relative to the upper end (Height is added to Z). */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ladder") FVector TopExitOffset=FVector(-110,0,0);
 /** Floor position relative to the actor origin. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ladder") FVector BottomExitOffset=FVector(110,0,0);
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance") TObjectPtr<UStaticMesh> LadderMesh;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance") FTransform MeshTransform=FTransform::Identity;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance") TObjectPtr<UMaterialInterface> LadderMaterial;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ladder") TObjectPtr<UBoxComponent> InteractionTarget;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ladder") FText LastFailureReason;

 UFUNCTION(BlueprintCallable, CallInEditor) void Rebuild();
 UFUNCTION(BlueprintPure) bool CanInteract(ACharacter* Character) const;
 UFUNCTION(BlueprintPure) FText GetPrompt() const;
 UFUNCTION(BlueprintCallable) bool Interact(ACharacter* Character,APlayerController* Controller);
 /** Positive climbs up, negative climbs down. Call once per frame from the existing input owner. */
 UFUNCTION(BlueprintCallable) void ClimbAxis(float Axis,float DeltaSeconds);
 UFUNCTION(BlueprintPure) bool IsClimbing() const { return Climber.IsValid(); }
 UFUNCTION(BlueprintPure) ACharacter* GetClimber() const { return Climber.Get(); }
 /** Steps off only at a clear endpoint; never teleports a player off the middle of a ladder. */
 UFUNCTION(BlueprintCallable) bool Cancel();

private:
 enum class ETraversal : uint8 { None, Aligning, Climbing, Dismounting };
 ETraversal Traversal=ETraversal::None;
 UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
 TWeakObjectPtr<ACharacter> Climber;
 TWeakObjectPtr<APlayerController> ClimbController;
 TArray<FVector> Route;
 int32 RouteIndex=0;
 uint8 SavedMovementMode=0;
 uint8 SavedCustomMovementMode=0;
 bool bAddedInputIgnore=false;
 bool bAddedActorIgnore=false;
 bool bAddedClimbingTag=false;
 uint64 LastInteractionFrame=MAX_uint64;
 float LowerZ=0,UpperZ=0;

 FVector EndAnchor(ACharacter* Character,bool bTop) const;
 bool FindLanding(ACharacter* Character,bool bTop,FVector& OutCenter) const;
 bool CapsuleFits(ACharacter* Character,const FVector& Center) const;
 bool ClearPath(ACharacter* Character,const TArray<FVector>& Points) const;
 bool BeginDismount(bool bTop);
 bool MoveSwept(const FVector& Target);
 void AdvanceRoute(float Distance);
 void RestorePlayer();
 UStaticMeshComponent* AddPart(FName Name,const FVector& Location,const FVector& Size);
};
