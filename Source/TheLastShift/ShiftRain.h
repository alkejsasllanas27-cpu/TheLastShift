#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShiftRain.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class USoundBase;
class UAudioComponent;
class UDirectionalLightComponent;
class USkyLightComponent;

/**
 * Night rain around the player.
 *  - Four open tubes of falling streaks at different distances (parallax), in lit translucency (volumetric,
 *    lit from every side like particles), so drops catch the flashlight beam, the lamps and the lightning.
 *  - A rain loop outside that turns into muffled drumming on the roof indoors.
 *  - Lightning: the moonlight, sky light and sky dome flare for a fraction of a second, then thunder rolls in
 *    after a delay (muffled indoors). Through windows, the strike lights the rooms too.
 * The director feeds it the camera position and whether there is a roof overhead.
 */
UCLASS()
class THELASTSHIFT_API AShiftRain : public AActor
{
	GENERATED_BODY()

public:
	AShiftRain();
	virtual void Tick(float DeltaSeconds) override;

	/** Where the camera is and whether it is under a roof; the director calls this every frame. */
	void SetViewer(const FVector& CameraLocation, bool bUnderRoof);

	UPROPERTY(EditAnywhere, Category = "Rain") TObjectPtr<UMaterialInterface> RainMaterial;
	UPROPERTY(EditAnywhere, Category = "Rain") TObjectPtr<USoundBase> RainOutsideSound;
	UPROPERTY(EditAnywhere, Category = "Rain") TObjectPtr<USoundBase> RainInsideSound;
	UPROPERTY(EditAnywhere, Category = "Rain") TArray<TObjectPtr<USoundBase>> ThunderSounds;

	/** Overall strength of the falling streaks (scales each layer's opacity). */
	UPROPERTY(EditAnywhere, Category = "Rain", meta = (ClampMin = "0")) float Intensity = 1.f;
	/** How far the rain leans with the wind, and which way it blows. */
	UPROPERTY(EditAnywhere, Category = "Rain") float WindTiltDegrees = 8.f;
	UPROPERTY(EditAnywhere, Category = "Rain") float WindYaw = 35.f;
	UPROPERTY(EditAnywhere, Category = "Rain", meta = (ClampMin = "0")) float RainVolume = .5f;

	UPROPERTY(EditAnywhere, Category = "Rain|Lightning", meta = (ClampMin = "1")) float LightningMinGap = 22.f;
	UPROPERTY(EditAnywhere, Category = "Rain|Lightning", meta = (ClampMin = "1")) float LightningMaxGap = 55.f;
	/** Moonlight multiplier at the brightest moment of a strike. */
	UPROPERTY(EditAnywhere, Category = "Rain|Lightning", meta = (ClampMin = "0")) float LightningBoost = 30.f;
	UPROPERTY(EditAnywhere, Category = "Rain|Lightning", meta = (ClampMin = "0")) float ThunderVolume = 1.f;

	/** Fires a lightning strike now (thunder follows). */
	UFUNCTION(BlueprintCallable, Category = "Rain|Lightning") void Strike();

protected:
	virtual void BeginPlay() override;

private:
	void BuildTube(UProceduralMeshComponent* Mesh, float Radius, float Height, int32 Segments) const;
	/** 0..1 brightness of a strike, T seconds after it began: a hard flash, a flicker, a second flash, an afterglow. */
	static float StrikeEnvelope(float T);

	UPROPERTY() TObjectPtr<USceneComponent> Root;
	UPROPERTY() TArray<TObjectPtr<UProceduralMeshComponent>> RainLayers;
	UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> LayerMaterials;
	UPROPERTY() TObjectPtr<UAudioComponent> OutsideAudio;
	UPROPERTY() TObjectPtr<UAudioComponent> InsideAudio;
	UPROPERTY() TObjectPtr<UDirectionalLightComponent> Moon;
	UPROPERTY() TObjectPtr<USkyLightComponent> Sky;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> SkyDome;

	TArray<float> LayerOpacity;
	float MoonBase = 0.f, SkyBase = 0.f, DomeBase = 0.f;
	FVector Viewer = FVector::ZeroVector;
	bool bUnderRoof = false;
	/** 1 out in the open, 0 under a roof; eased so stepping through a door fades rather than cuts. */
	float Openness = 1.f;
	float Clock = 0.f, NextStrike = 0.f, StrikeStart = -100.f, ThunderAt = -1.f;
	bool bFlashLit = false;
	int32 ThunderIndex = 0;
};
