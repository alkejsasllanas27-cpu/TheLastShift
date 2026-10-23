#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShiftApparition.generated.h"

class APawn;
class APlayerController;
class UAudioComponent;
class UMaterialInstanceDynamic;
class UPointLightComponent;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EShiftApparitionPhase : uint8
{
 Dormant,
 Appearing,
 Haunting,
 Vanishing,
 CoolingDown
};

/** A non-blocking hospital scare. It has no attack, catch, damage, or player-control behavior. */
UCLASS()
class THELASTSHIFT_API AShiftApparition : public AActor
{
 GENERATED_BODY()

public:
 AShiftApparition();
 virtual void OnConstruction(const FTransform& Transform) override;
 virtual void BeginPlay() override;
 virtual void Tick(float DeltaSeconds) override;

 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Apparition") TObjectPtr<USceneComponent> SceneRoot;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Apparition") TObjectPtr<UStaticMeshComponent> Creature;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Apparition") TObjectPtr<UAudioComponent> Voice;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Apparition") TObjectPtr<UPointLightComponent> ApparitionLight;

 /** Ghosts appear and drift a short distance; nurses remain at their placed position. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition") bool bGhost=true;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition") bool bNurseVisibleWhileIdle=true;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition") TObjectPtr<USoundBase> ScareSound;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="0", ClampMax="1")) float ScareVolume=.65f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="400")) float TriggerRadius=1350.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="250")) float MinTriggerDistance=350.f;
 /** Visibility and drift stop before this distance; runtime always enforces at least 220 cm. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="220")) float SafeDistance=240.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="-1", ClampMax="1")) float LookDotThreshold=.48f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="1.3", ClampMax="8")) float ScareSeconds=3.6f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="5")) float CooldownSeconds=18.f;
 /** Player must also leave the trigger radius by this amount before another scare can arm. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="100")) float RearmMargin=250.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="0", ClampMax="200")) float GhostTravelDistance=160.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="0", ClampMax="100")) float GhostDriftSpeed=48.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="0", ClampMax="20")) float GhostFloatHeight=9.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition", meta=(ClampMin="0", ClampMax="100")) float LightIntensity=12.f;
 /** Optional scalar in supplied materials. Visibility still works with ordinary opaque materials. */
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apparition") FName OpacityParameter=TEXT("ApparitionOpacity");

 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Apparition|Runtime") int32 ScareCount=0;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Apparition|Runtime") bool bActive=false;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Apparition|Runtime") bool bArmed=true;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Apparition|Runtime") EShiftApparitionPhase Phase=EShiftApparitionPhase::Dormant;
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Apparition|Runtime") float CooldownRemaining=0.f;

private:
 UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;
 FVector HomeLocation=FVector::ZeroVector;
 FRotator HomeRotation=FRotator::ZeroRotator;
 FVector VisualOrigin=FVector::ZeroVector;
 FRotator VisualRotation=FRotator::ZeroRotator;
 FVector DriftDirection=FVector::ZeroVector;
 float Age=0.f;
 float EncounterAge=0.f;
 bool bLeftTriggerArea=false;

 void EnforceNoCollision();
 void SetVisualOpacity(float Opacity);
 void UpdateVisualSway(float Strength);
 bool PlayerIsLooking(APlayerController* Controller, APawn* Player) const;
 void BeginEncounter(APawn* Player);
 void EndEncounter();
 float MinimumSafeDistance() const;
};
