#include "ShiftApparition.h"

#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"

AShiftApparition::AShiftApparition()
{
 PrimaryActorTick.bCanEverTick=true;
 SetActorEnableCollision(false);

 SceneRoot=CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
 SetRootComponent(SceneRoot);
 SceneRoot->SetMobility(EComponentMobility::Movable);

 Creature=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Creature"));
 Creature->SetupAttachment(SceneRoot);
 Creature->SetMobility(EComponentMobility::Movable);
 Creature->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Creature->SetCollisionResponseToAllChannels(ECR_Ignore);
 Creature->SetGenerateOverlapEvents(false);
 Creature->SetCanEverAffectNavigation(false);
 Creature->SetCastShadow(false);

 Voice=CreateDefaultSubobject<UAudioComponent>(TEXT("Voice"));
 Voice->SetupAttachment(SceneRoot);
 Voice->SetRelativeLocation(FVector(0,0,140));
 Voice->bAutoActivate=false;
 Voice->bOverrideAttenuation=true;
 Voice->AttenuationOverrides.bAttenuate=true;
 Voice->AttenuationOverrides.bSpatialize=true;
 Voice->AttenuationOverrides.AttenuationShapeExtents=FVector(150.f);
 Voice->AttenuationOverrides.FalloffDistance=1650.f;
 Voice->SetVolumeMultiplier(ScareVolume);

 ApparitionLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("ApparitionLight"));
 ApparitionLight->SetupAttachment(SceneRoot);
 ApparitionLight->SetRelativeLocation(FVector(25,0,115));
 ApparitionLight->SetIntensityUnits(ELightUnits::Candelas);
 ApparitionLight->SetIntensity(LightIntensity);
 ApparitionLight->SetAttenuationRadius(330.f);
 ApparitionLight->SetCastShadows(false);
 ApparitionLight->SetVisibility(false);
}

void AShiftApparition::EnforceNoCollision()
{
 // Includes any additional visual components supplied by a derived Blueprint.
 SetActorEnableCollision(false);
 TInlineComponentArray<UPrimitiveComponent*> Primitives(this);
 for(UPrimitiveComponent* Primitive:Primitives)
 {
  Primitive->SetSimulatePhysics(false);
  Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  Primitive->SetCollisionResponseToAllChannels(ECR_Ignore);
  Primitive->SetGenerateOverlapEvents(false);
  Primitive->SetCanEverAffectNavigation(false);
 }
}

void AShiftApparition::OnConstruction(const FTransform& Transform)
{
 Super::OnConstruction(Transform);
 EnforceNoCollision();
 ApparitionLight->SetLightColor(bGhost?FLinearColor(.32f,.68f,1.f):FLinearColor(1.f,.23f,.18f));
 ApparitionLight->SetIntensity(FMath::Max(0.f,LightIntensity));
 // Both meshes remain visible for positioning in the editor; BeginPlay applies game visibility.
}

void AShiftApparition::BeginPlay()
{
 Super::BeginPlay();
 EnforceNoCollision();
 HomeLocation=GetActorLocation();
 HomeRotation=GetActorRotation();
 VisualOrigin=Creature->GetRelativeLocation();
 VisualRotation=Creature->GetRelativeRotation();
 for(int32 Index=0;Index<Creature->GetNumMaterials();++Index)
 {
  if(UMaterialInstanceDynamic* Material=Creature->CreateDynamicMaterialInstance(Index))
   DynamicMaterials.Add(Material);
 }
 bActive=false;
 bArmed=true;
 Phase=EShiftApparitionPhase::Dormant;
 ScareCount=0;
 CooldownRemaining=0.f;
 SetVisualOpacity(!bGhost && bNurseVisibleWhileIdle?1.f:0.f);
}

float AShiftApparition::MinimumSafeDistance() const
{
 return FMath::Max(220.f,SafeDistance);
}

void AShiftApparition::SetVisualOpacity(float Opacity)
{
 const float Alpha=FMath::Clamp(Opacity,0.f,1.f);
 const bool bVisible=Alpha>.005f;
 Creature->SetVisibility(bVisible,true);
 Creature->SetHiddenInGame(!bVisible,true);
 for(UMaterialInstanceDynamic* Material:DynamicMaterials)
 {
  if(Material && !OpacityParameter.IsNone())
   Material->SetScalarParameterValue(OpacityParameter,Alpha);
 }
 const float Pulse=bActive?.72f+.28f*FMath::Sin(EncounterAge*17.f):.2f;
 ApparitionLight->SetVisibility(bVisible);
 ApparitionLight->SetIntensity(FMath::Max(0.f,LightIntensity)*Alpha*Pulse);
}

void AShiftApparition::UpdateVisualSway(float Strength)
{
 FRotator Sway=VisualRotation;
 Sway.Roll+=FMath::Sin(Age*1.5f)*1.5f*Strength;
 Sway.Pitch+=FMath::Sin(Age*1.1f)*.65f*Strength;
 Creature->SetRelativeRotation(Sway);
 const float Bob=bGhost?FMath::Sin(Age*1.7f)*FMath::Clamp(GhostFloatHeight,0.f,20.f):FMath::Sin(Age*1.3f)*.8f;
 Creature->SetRelativeLocation(VisualOrigin+FVector(0,0,Bob*Strength));
}

bool AShiftApparition::PlayerIsLooking(APlayerController* Controller, APawn* Player) const
{
 if(!Controller || !Player || !GetWorld()) return false;
 FVector Eye;
 FRotator View;
 Controller->GetPlayerViewPoint(Eye,View);
 const FVector Aim=Creature->Bounds.Origin;
 const FVector Toward=(Aim-Eye).GetSafeNormal();
 if(FVector::DotProduct(View.Vector(),Toward)<FMath::Clamp(LookDotThreshold,-1.f,1.f)) return false;
 FCollisionQueryParams Params(SCENE_QUERY_STAT(ApparitionSight),true,this);
 Params.AddIgnoredActor(Player);
 FHitResult Hit;
 return !GetWorld()->LineTraceSingleByChannel(Hit,Eye,Aim,ECC_Visibility,Params);
}

void AShiftApparition::BeginEncounter(APawn* Player)
{
 if(!Player || !bArmed || bActive) return;
 bActive=true;
 bArmed=false;
 bLeftTriggerArea=false;
 EncounterAge=0.f;
 Phase=EShiftApparitionPhase::Appearing;
 ++ScareCount;
 // A single short drift towards the initial position, never pursuit of a moving player.
 DriftDirection=(Player->GetActorLocation()-HomeLocation).GetSafeNormal2D();
 if(ScareSound)
 {
  Voice->Stop();
  Voice->SetSound(ScareSound);
  Voice->SetVolumeMultiplier(FMath::Clamp(ScareVolume,0.f,1.f));
  Voice->SetPitchMultiplier(bGhost?.87f:1.02f);
  Voice->Play();
 }
}

void AShiftApparition::EndEncounter()
{
 bActive=false;
 Phase=EShiftApparitionPhase::CoolingDown;
 CooldownRemaining=FMath::Max(5.f,CooldownSeconds);
 SetVisualOpacity(0.f);
 if(Voice->IsPlaying()) Voice->FadeOut(.25f,0.f);
 // Only our own visual actor is moved. No pawn, controller, or camera is changed.
 SetActorLocationAndRotation(HomeLocation,HomeRotation,false,nullptr,ETeleportType::TeleportPhysics);
 Creature->SetRelativeLocation(VisualOrigin);
 Creature->SetRelativeRotation(VisualRotation);
}

void AShiftApparition::Tick(float DeltaSeconds)
{
 Super::Tick(DeltaSeconds);
 const float Dt=FMath::Max(0.f,DeltaSeconds);
 Age+=Dt;
 // Even a runtime collision toggle cannot turn this actor into a blocker.
 SetActorEnableCollision(false);

 APlayerController* Controller=UGameplayStatics::GetPlayerController(this,0);
 APawn* Player=Controller?Controller->GetPawn():nullptr;
 if(!Player)
 {
  if(bActive) EndEncounter();
  SetVisualOpacity(0.f);
  return;
 }

 const float HomeDistance=FVector::Dist2D(Player->GetActorLocation(),HomeLocation);
 const float FloorDifference=FMath::Abs(Player->GetActorLocation().Z-(HomeLocation.Z+90.f));
 const float Radius=FMath::Max(MinimumSafeDistance()+150.f,TriggerRadius);
 if(!bArmed && HomeDistance>Radius+FMath::Max(100.f,RearmMargin)) bLeftTriggerArea=true;

 if(!bActive)
 {
  CooldownRemaining=FMath::Max(0.f,CooldownRemaining-Dt);
  if(!bArmed && CooldownRemaining<=0.f && bLeftTriggerArea)
  {
   bArmed=true;
   Phase=EShiftApparitionPhase::Dormant;
  }
  const bool bIdleNurseVisible=!bGhost && bNurseVisibleWhileIdle && bArmed && HomeDistance>MinimumSafeDistance();
  UpdateVisualSway(.6f);
  SetVisualOpacity(bIdleNurseVisible?1.f:0.f);
  if(bArmed && FloorDifference<230.f && HomeDistance<=Radius && HomeDistance>FMath::Max(MinTriggerDistance,MinimumSafeDistance()+60.f)
    && PlayerIsLooking(Controller,Player))
   BeginEncounter(Player);
  else
   return;
 }

 const float Distance=FVector::Dist2D(Player->GetActorLocation(),GetActorLocation());
 const float Duration=FMath::Clamp(ScareSeconds,1.3f,8.f);
 EncounterAge+=Dt;
 if(Distance<=MinimumSafeDistance() || EncounterAge>=Duration || HomeDistance>Radius*1.6f || FloorDifference>350.f)
 {
  EndEncounter();
  return;
 }

 if(bGhost)
 {
  const float Travel=FMath::Min(FMath::Clamp(GhostTravelDistance,0.f,200.f),EncounterAge*FMath::Clamp(GhostDriftSpeed,0.f,100.f));
  FVector Candidate=HomeLocation+DriftDirection*Travel;
  // Fade away rather than approach a player who steps across the drift path.
  if(FVector::Dist2D(Player->GetActorLocation(),Candidate)<=MinimumSafeDistance())
  {
   EndEncounter();
   return;
  }
  FCollisionQueryParams Params(SCENE_QUERY_STAT(ApparitionDrift),false,this);
  Params.AddIgnoredActor(Player);
  const FVector Origin=Creature->Bounds.Origin;
  FHitResult Wall;
  if(GetWorld()->LineTraceSingleByChannel(Wall,Origin,Origin+(Candidate-GetActorLocation()),ECC_Visibility,Params))
   Candidate=GetActorLocation();
  SetActorLocation(Candidate,false,nullptr,ETeleportType::TeleportPhysics);
 }
 else
 {
  // The nurse turns in place; her feet never leave the assigned hospital post.
  FRotator Facing=HomeRotation;
  Facing.Yaw=(Player->GetActorLocation()-HomeLocation).Rotation().Yaw;
  SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(),Facing,Dt,55.f));
 }

 const float FadeIn=.2f;
 const float FadeOut=.7f;
 float Alpha=1.f;
 if(EncounterAge<FadeIn)
 {
  Phase=EShiftApparitionPhase::Appearing;
  Alpha=EncounterAge/FadeIn;
 }
 else if(EncounterAge>Duration-FadeOut)
 {
  Phase=EShiftApparitionPhase::Vanishing;
  Alpha=(Duration-EncounterAge)/FadeOut;
 }
 else
  Phase=EShiftApparitionPhase::Haunting;
 UpdateVisualSway(1.f);
 SetVisualOpacity(Alpha);
}
