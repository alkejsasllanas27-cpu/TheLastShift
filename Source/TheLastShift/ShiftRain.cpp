#include "ShiftRain.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/AudioComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** One tube of rain: radius, streak tiling around and down, fall speed (tiles per second), opacity.
	 *  Near streaks are big and fast; far ones fine and dense, which gives the rain its depth. */
	struct FRainLayerSpec { float Radius, TilingU, TilingV, Speed, Opacity; };
	const FRainLayerSpec RainLayerSpecs[] = {
		{ 120.f, 2.f, 6.f, 2.9f, .5f },     // right in front of the camera, so rain shows even facing a wall
		{ 280.f, 3.f, 7.f, 2.7f, .55f },
		{ 700.f, 8.f, 9.f, 2.4f, .45f },
		{ 1600.f, 18.f, 12.f, 2.1f, .35f },
	};
	constexpr float TubeHeight = 3200.f;
	/** Thunder delay after the flash for each sound (close, rolling, distant), and how bright that strike is. */
	const float ThunderDelay[] = { .4f, 1.6f, 3.4f };
	const float StrikeStrength[] = { 1.f, .7f, .4f };
}

AShiftRain::AShiftRain()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	for (int32 i = 0; i < UE_ARRAY_COUNT(RainLayerSpecs); ++i)
	{
		UProceduralMeshComponent* Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(*FString::Printf(TEXT("RainLayer%d"), i));
		Mesh->SetupAttachment(Root);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->CastShadow = false;
		// The nearest layer draws last, over the far ones.
		Mesh->TranslucencySortPriority = 10 - i;
		RainLayers.Add(Mesh);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/ShiftAssets/Weather/M_RainStreaks.M_RainStreaks"));
	if (Mat.Succeeded()) RainMaterial = Mat.Object;
	static ConstructorHelpers::FObjectFinder<USoundBase> Outside(TEXT("/Game/ShiftAssets/Weather/Rain_Outside_Loop.Rain_Outside_Loop"));
	if (Outside.Succeeded()) RainOutsideSound = Outside.Object;
	static ConstructorHelpers::FObjectFinder<USoundBase> Inside(TEXT("/Game/ShiftAssets/Weather/Rain_Inside_Loop.Rain_Inside_Loop"));
	if (Inside.Succeeded()) RainInsideSound = Inside.Object;
	static ConstructorHelpers::FObjectFinder<USoundBase> Close(TEXT("/Game/ShiftAssets/Weather/Thunder_Close.Thunder_Close"));
	static ConstructorHelpers::FObjectFinder<USoundBase> Rolling(TEXT("/Game/ShiftAssets/Weather/Thunder_Rolling.Thunder_Rolling"));
	static ConstructorHelpers::FObjectFinder<USoundBase> Distant(TEXT("/Game/ShiftAssets/Weather/Thunder_Distant.Thunder_Distant"));
	ThunderSounds = { Close.Object, Rolling.Object, Distant.Object };
}

void AShiftRain::BeginPlay()
{
	Super::BeginPlay();
	for (int32 i = 0; i < RainLayers.Num(); ++i)
	{
		const FRainLayerSpec& Spec = RainLayerSpecs[i];
		BuildTube(RainLayers[i], Spec.Radius, TubeHeight, i == 0 ? 32 : 48);
		UMaterialInstanceDynamic* Mid = RainMaterial ? UMaterialInstanceDynamic::Create(RainMaterial, this) : nullptr;
		if (Mid)
		{
			Mid->SetScalarParameterValue(TEXT("TilingU"), Spec.TilingU);
			Mid->SetScalarParameterValue(TEXT("TilingV"), Spec.TilingV);
			Mid->SetScalarParameterValue(TEXT("Speed"), Spec.Speed);
			Mid->SetScalarParameterValue(TEXT("Opacity"), 0.f);
			Mid->SetScalarParameterValue(TEXT("Glow"), .07f);
			RainLayers[i]->SetMaterial(0, Mid);
		}
		LayerMaterials.Add(Mid);
		LayerOpacity.Add(Spec.Opacity);
	}
	// The rain leans with the wind; only the position follows the player afterwards.
	Root->SetWorldRotation(FRotator(WindTiltDegrees, WindYaw, 0.f));

	if (RainOutsideSound) OutsideAudio = UGameplayStatics::SpawnSound2D(this, RainOutsideSound, .001f, 1.f, 0.f, nullptr, false, false);
	if (RainInsideSound) InsideAudio = UGameplayStatics::SpawnSound2D(this, RainInsideSound, .001f, 1.f, 0.f, nullptr, false, false);

	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It) { Moon = Cast<UDirectionalLightComponent>(It->GetLightComponent()); break; }
	if (Moon) MoonBase = Moon->Intensity;
	for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It) { Sky = It->GetLightComponent(); break; }
	if (Sky) SkyBase = Sky->Intensity;
	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
	{
		UStaticMeshComponent* Mesh = It->GetStaticMeshComponent();
		if (Mesh && Mesh->GetStaticMesh() && Mesh->GetStaticMesh()->GetName() == TEXT("SM_SkySphere"))
		{
			SkyDome = Mesh->CreateAndSetMaterialInstanceDynamic(0);
			if (SkyDome) DomeBase = SkyDome->K2_GetScalarParameterValue(TEXT("SkyBrightness"));
			break;
		}
	}
	NextStrike = FMath::FRandRange(10.f, 18.f);
}

void AShiftRain::BuildTube(UProceduralMeshComponent* Mesh, float Radius, float Height, int32 Segments) const
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	for (int32 i = 0; i <= Segments; ++i)
	{
		const float Angle = 2.f * PI * i / Segments;
		const FVector Out(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
		for (int32 Row = 0; Row < 2; ++Row)
		{
			// Row 0 is the top (V = 0): the streaks scroll toward V = 1, downward.
			Vertices.Add(Out * Radius + FVector(0.f, 0.f, Row == 0 ? Height * .5f : -Height * .5f));
			Normals.Add(-Out);
			UVs.Add(FVector2D(static_cast<float>(i) / Segments, Row));
			Tangents.Add(FProcMeshTangent(FVector(-Out.Y, Out.X, 0.f), false));
		}
	}
	for (int32 i = 0; i < Segments; ++i)
	{
		const int32 A = i * 2;
		Triangles.Append({ A, A + 2, A + 1, A + 1, A + 2, A + 3 });
	}
	Mesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), Tangents, false);
}

void AShiftRain::SetViewer(const FVector& CameraLocation, bool bInUnderRoof)
{
	Viewer = CameraLocation;
	bUnderRoof = bInUnderRoof;
}

float AShiftRain::StrikeEnvelope(float T)
{
	if (T < 0.f || T > 1.4f) return 0.f;
	auto Pulse = [T](float Start, float Length, float Peak)
	{
		const float U = (T - Start) / Length;
		return (U >= 0.f && U <= 1.f) ? Peak * (1.f - U * U) : 0.f;
	};
	float E = FMath::Max3(Pulse(0.f, .07f, 1.f), Pulse(.11f, .05f, .45f), Pulse(.21f, .17f, .85f));
	if (T > .21f) E = FMath::Max(E, .14f * FMath::Exp(-(T - .21f) * 4.5f));
	return E;
}

void AShiftRain::Strike()
{
	StrikeStart = Clock;
	NextStrike = Clock + FMath::FRandRange(LightningMinGap, FMath::Max(LightningMinGap, LightningMaxGap));
	ThunderIndex = FMath::RandRange(0, 2);
	ThunderAt = Clock + ThunderDelay[ThunderIndex] + FMath::FRandRange(0.f, .7f);
}

void AShiftRain::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Clock += DeltaSeconds;
	SetActorLocation(Viewer);

	// Under a roof the streaks fade away and the rain turns into drumming on the roof.
	Openness = FMath::FInterpConstantTo(Openness, bUnderRoof ? 0.f : 1.f, DeltaSeconds, 2.5f);
	for (int32 i = 0; i < RainLayers.Num(); ++i)
	{
		const float Opacity = LayerOpacity[i] * Intensity * Openness;
		if (LayerMaterials[i]) LayerMaterials[i]->SetScalarParameterValue(TEXT("Opacity"), Opacity);
		RainLayers[i]->SetVisibility(Opacity > .001f);
	}
	if (OutsideAudio) OutsideAudio->SetVolumeMultiplier(FMath::Max(.001f, RainVolume * Openness));
	if (InsideAudio) InsideAudio->SetVolumeMultiplier(FMath::Max(.001f, RainVolume * .9f * (1.f - Openness)));

	if (Clock >= NextStrike) Strike();
	const float E = StrikeEnvelope(Clock - StrikeStart) * StrikeStrength[ThunderIndex];
	// Lights are only touched while a strike is lit, and put back exactly once it is over (even at a low frame rate).
	if (E > 0.f || bFlashLit)
	{
		if (Moon) Moon->SetIntensity(MoonBase * (1.f + LightningBoost * E));
		if (Sky) Sky->SetIntensity(SkyBase * (1.f + LightningBoost * .35f * E));
		if (SkyDome) SkyDome->SetScalarParameterValue(TEXT("SkyBrightness"), DomeBase * (1.f + 14.f * E));
		bFlashLit = E > 0.f;
	}
	if (ThunderAt > 0.f && Clock >= ThunderAt)
	{
		ThunderAt = -1.f;
		if (ThunderSounds.IsValidIndex(ThunderIndex) && ThunderSounds[ThunderIndex])
		{
			// Heard through walls, thunder loses its crack and keeps the rumble.
			UAudioComponent* Thunder = UGameplayStatics::SpawnSound2D(this, ThunderSounds[ThunderIndex], ThunderVolume * (Openness > .5f ? 1.f : .75f));
			if (Thunder && Openness <= .5f)
			{
				Thunder->SetLowPassFilterEnabled(true);
				Thunder->SetLowPassFilterFrequency(650.f);
			}
		}
	}
}
