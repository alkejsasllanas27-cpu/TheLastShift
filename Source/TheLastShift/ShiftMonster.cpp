#include "ShiftMonster.h"
#include "ShiftGameplay.h"
#include "AIController.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SpotLightComponent.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

AShiftMonster::AShiftMonster()
{
 PrimaryActorTick.bCanEverTick=true;
 GetCapsuleComponent()->InitCapsuleSize(60.f,105.f);
 bUseControllerRotationYaw=false;
 AutoPossessAI=EAutoPossessAI::PlacedInWorldOrSpawned;
 AIControllerClass=AAIController::StaticClass();
 auto* Movement=GetCharacterMovement();
 Movement->MaxWalkSpeed=ChaseSpeed;
 Movement->MaxStepHeight=42.f;
 Movement->MaxAcceleration=700.f;
 Movement->BrakingDecelerationWalking=1100.f;
 Movement->bOrientRotationToMovement=false;
 Creature=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Creature"));
 Creature->SetupAttachment(GetCapsuleComponent());
 Creature->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> MonsterMesh(TEXT("/Game/ShiftAssets/ScaryMonster/SM_ScaryMonster.SM_ScaryMonster"));
 if(MonsterMesh.Succeeded()) Creature->SetStaticMesh(MonsterMesh.Object);
 Creature->SetVisibility(false,true);
 Creature->SetHiddenInGame(true);
 static ConstructorHelpers::FObjectFinder<USkeletalMesh> RiggedMonster(TEXT("/Game/ShiftAssets/ScaryMonster/Rigged/SK_ScaryMonster.SK_ScaryMonster"));
 static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkClip(TEXT("/Game/ShiftAssets/ScaryMonster/Rigged/A_Monster_Walk.A_Monster_Walk"));
 static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleClip(TEXT("/Game/ShiftAssets/ScaryMonster/Rigged/A_Monster_Idle.A_Monster_Idle"));
 if(RiggedMonster.Succeeded()) GetMesh()->SetSkeletalMeshAsset(RiggedMonster.Object);
 WalkAnimation=WalkClip.Object;IdleAnimation=IdleClip.Object;
 GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
 Voice=CreateDefaultSubobject<UAudioComponent>(TEXT("Voice"));
 Voice->SetupAttachment(GetCapsuleComponent());
 Voice->SetupAttachment(GetMesh(),TEXT("head"));
 Voice->SetRelativeLocation(FVector::ZeroVector);
 Voice->bAutoActivate=false;
 Voice->bOverrideAttenuation=true;
 Voice->AttenuationOverrides.bAttenuate=true;
 Voice->AttenuationOverrides.bSpatialize=true;
 Voice->AttenuationOverrides.AttenuationShapeExtents=FVector(250.f);
 Voice->AttenuationOverrides.FalloffDistance=2400.f;
 Voice->SetVolumeMultiplier(.7f);

 // Stays hidden until the grab, then it is swung in front of the camera so the room
 // disappears and there is nothing on screen but the creature and red.
 ScareBackdrop=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScareBackdrop"));
 ScareBackdrop->SetupAttachment(GetCapsuleComponent());
 ScareBackdrop->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 ScareBackdrop->SetCastShadow(false);
 ScareBackdrop->SetVisibility(false);
 static ConstructorHelpers::FObjectFinder<UStaticMesh> BackdropMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
 if(BackdropMesh.Succeeded()) ScareBackdrop->SetStaticMesh(BackdropMesh.Object);
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> BackdropMaterial(TEXT("/Game/ShiftAssets/Materials/M_JumpscareBackdrop.M_JumpscareBackdrop"));
 if(BackdropMaterial.Succeeded()) ScareBackdrop->SetMaterial(0,BackdropMaterial.Object);

 ScareLight=CreateDefaultSubobject<USpotLightComponent>(TEXT("ScareLight"));
 ScareLight->SetupAttachment(GetCapsuleComponent());
 ScareLight->SetIntensityUnits(ELightUnits::Candelas);
 ScareLight->SetIntensity(95.f);
 ScareLight->SetAttenuationRadius(700.f);
 ScareLight->SetInnerConeAngle(30.f);
 ScareLight->SetOuterConeAngle(60.f);
 ScareLight->SetLightColor(FLinearColor(1.f,.72f,.66f));
 ScareLight->SetCastShadows(false);
 ScareLight->SetVisibility(false);
}

void AShiftMonster::OnConstruction(const FTransform& Transform)
{
 Super::OnConstruction(Transform);
 // Original anatomy is quadrupedal: Z is up, and the head points along -Y in the imported mesh.
 const FRotator Pose(0,90,0);
 const FVector Scale(.70f);
 Creature->SetVisibility(false,true);Creature->SetHiddenInGame(true);
 GetMesh()->SetRelativeRotation(Pose);
 GetMesh()->SetRelativeScale3D(Scale);
 if(USkeletalMesh* MonsterMesh=GetMesh()->GetSkeletalMeshAsset())
 {
  const FBox Box=MonsterMesh->GetBounds().GetBox().TransformBy(FTransform(Pose,FVector::ZeroVector,Scale));
  VisualOrigin=FVector(-Box.GetCenter().X,-Box.GetCenter().Y,-GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()-Box.Min.Z);
  GetMesh()->SetRelativeLocation(VisualOrigin);
 }
}

void AShiftMonster::BeginPlay()
{
 Super::BeginPlay();
 VisualOrigin=GetMesh()->GetRelativeLocation();
 if(IdleAnimation) GetMesh()->PlayAnimation(IdleAnimation,true);
 HomeLocation=GetActorLocation();PatrolTarget=HomeLocation;PatrolPause=FMath::FRandRange(1.f,4.f);
 GetCharacterMovement()->MaxWalkSpeed=PatrolSpeed;
 if(!Controller) SpawnDefaultController();
}

bool AShiftMonster::CanSee(APawn* Target) const
{
 FHitResult Hit;
 FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterSight),true,this);
 const FVector Eye=GetMesh()->GetSocketLocation(TEXT("head"));
 const FVector Aim=Target->GetActorLocation()+FVector(0,0,45);
 return !GetWorld()->LineTraceSingleByChannel(Hit,Eye,Aim,ECC_Visibility,Params) || Hit.GetActor()==Target;
}

FVector AShiftMonster::Steer(const FVector& Desired, APawn* Target) const
{
 FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterAvoidance),false,this);
 Params.AddIgnoredActor(Target);
 float Best=-10000.f;
 FVector Result=FVector::ZeroVector;
 // Short swept probes steer around nearby obstacles; CharacterMovement supplies ground following and collision.
 for(float Angle : {0.f,35.f,-35.f,70.f,-70.f,110.f,-110.f})
 {
  const FVector Dir=Desired.RotateAngleAxis(Angle,FVector::UpVector);
  FHitResult Wall,Floor;
  const FVector Start=GetActorLocation();
  const bool Blocked=GetWorld()->SweepSingleByChannel(Wall,Start,Start+Dir*190.f,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(61.f,80.f),Params);
  const float Clearance=Blocked?Wall.Time:1.f;
  const FVector Ahead=Start+Dir*120.f;
  const bool HasFloor=GetWorld()->LineTraceSingleByChannel(Floor,Ahead,Ahead-FVector(0,0,220.f),ECC_Visibility,Params);
  if(Clearance<.22f || !HasFloor || Floor.ImpactNormal.Z<.65f) continue;
  const float Score=FVector::DotProduct(Desired,Dir)*1.5f+Clearance*2.f;
  if(Score>Best){Best=Score;Result=Dir;}
 }
 return Result;
}

void AShiftMonster::Roar()
{
 if(Scream && ScreamCooldown<=0.f)
 {
  Voice->Stop();Voice->SetSound(Scream);Voice->SetPitchMultiplier(FMath::FRandRange(.91f,1.03f));Voice->Play();
  ScreamCooldown=9.f;++ScreamCount;
 }
}

void AShiftMonster::ChoosePatrolTarget()
{
 for(int32 Attempt=0;Attempt<8;++Attempt)
 {
  const float Angle=FMath::FRandRange(0.f,2.f*PI);
  const float Distance=FMath::FRandRange(PatrolRadius*.35f,PatrolRadius);
  const FVector Candidate=HomeLocation+FVector(FMath::Cos(Angle),FMath::Sin(Angle),0)*Distance;
  FHitResult Floor;
  FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterPatrol),false,this);
  if(GetWorld()->LineTraceSingleByChannel(Floor,Candidate+FVector(0,0,150.f),Candidate-FVector(0,0,400.f),ECC_Visibility,Params) && Floor.ImpactNormal.Z>.7f)
  {
   PatrolTarget=Floor.ImpactPoint+FVector(0,0,GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
   return;
  }
 }
 PatrolTarget=HomeLocation;
}

void AShiftMonster::Prowl(APawn* Target, float Dt)
{
 GetCharacterMovement()->MaxWalkSpeed=PatrolSpeed;
 // A running player is heard through walls; walking leaves it to its rounds.
 if(SearchTime<=0.f && Target->GetVelocity().Size2D()>HearingSpeed)
 {
  const float Noise=(Target->GetActorLocation()-GetActorLocation()).Size();
  if(Noise<HearingRadius){LastSeen=Target->GetActorLocation();SearchTime=SearchSeconds;}
 }
 FVector Goal=PatrolTarget;
 if(SearchTime>0.f)
 {
  SearchTime-=Dt;
  Goal=LastSeen;
  // Once it arrives at the noise it casts about for a moment before going back to its rounds.
  if((LastSeen-GetActorLocation()).Size2D()<170.f) SearchTime=FMath::Min(SearchTime,2.5f);
  if(SearchTime<=0.f) ChoosePatrolTarget();
 }
 else if(PatrolPause>0.f){PatrolPause-=Dt;GetCharacterMovement()->StopMovementImmediately();return;}
 else if(PatrolTarget.IsNearlyZero()||(PatrolTarget-GetActorLocation()).Size2D()<140.f)
 {
  ChoosePatrolTarget();PatrolPause=FMath::FRandRange(2.5f,6.f);return;
 }
 const FVector Desired=(Goal-GetActorLocation()).GetSafeNormal2D();
 if(Desired.IsNearlyZero())return;
 SetActorRotation(FMath::RInterpTo(GetActorRotation(),Desired.Rotation(),Dt,2.5f));
 const FVector Move=Steer(Desired,Target);
 if(Move.IsNearlyZero()){ChoosePatrolTarget();PatrolPause=1.f;return;}
 AddMovementInput(Move,1.f);
}

void AShiftMonster::CatchPlayer(APawn* Target)
{
 if(bCaughtPlayer) return;
 bCaughtPlayer=true;CatchTime=0.f;++AttackCount;
 for(TActorIterator<AShiftDirector> It(GetWorld());It;++It){It->PlayerCaught();break;}
 ScreamCooldown=0.f;Roar();
 // A second, unspatialised copy of the scream: the spatialised one alone is too polite for a grab.
 if(Scream) UGameplayStatics::PlaySound2D(this,Scream,1.6f,FMath::FRandRange(.94f,1.02f));
 ScareBackdrop->SetVisibility(true);
 ScareLight->SetVisibility(true);
 GetCharacterMovement()->StopMovementImmediately();
 GetCharacterMovement()->DisableMovement();
 // The player is held still and blind to input for the whole scare; the level reload undoes it.
 if(APlayerController* PC=Cast<APlayerController>(Target->GetController())) Target->DisableInput(PC);
 if(auto* Movement=Target->FindComponentByClass<UCharacterMovementComponent>())
 {Movement->StopMovementImmediately();Movement->DisableMovement();}
}

void AShiftMonster::JumpscareTick(APawn* Target, float Dt)
{
 CatchTime+=Dt;
 APlayerController* PC=Cast<APlayerController>(Target->GetController());
 FVector EyeLocation=Target->GetActorLocation()+FVector(0,0,60);FRotator EyeRotation=GetActorRotation();
 if(PC) PC->GetPlayerViewPoint(EyeLocation,EyeRotation);
 // Face the player, then shift the whole actor so the creature's head lands right in front of the eyes.
 const FVector ToPlayer=(EyeLocation-GetActorLocation()).GetSafeNormal2D();
 // Nose up: the creature walks with its head hung low, so without this the grab shows its back.
 if(!ToPlayer.IsNearlyZero()) SetActorRotation(FRotator(28.f,ToPlayer.Rotation().Yaw,0.f));
 const FVector HeadOffset=GetMesh()->GetSocketLocation(TEXT("head"))-GetActorLocation();
 // It rushes the last stretch rather than appearing at the face already: the movement is the scare.
 const float Lunge=FMath::Clamp(CatchTime/.22f,0.f,1.f);
 const float Reach=FMath::Lerp(JumpscareLungeFrom,JumpscareFaceDistance,Lunge*Lunge);
 const FVector Face=EyeLocation-ToPlayer*Reach-FVector(0,0,18.f);
 SetActorLocation(Face-HeadOffset,false,nullptr,ETeleportType::TeleportPhysics);
 if(WalkAnimation && !bWalkingAnimation){bWalkingAnimation=true;GetMesh()->PlayAnimation(WalkAnimation,true);}
 GetMesh()->SetPlayRate(2.4f);

 // The backdrop is parked between the creature and the room, square to the view, big enough to
 // swallow the frame; the light is thrown from the player's shoulder so the face is not a silhouette.
 const FVector HeadLocation=GetMesh()->GetSocketLocation(TEXT("head"));
 // Hung square to the view and wide enough to run past the edges of the frame at this distance.
 ScareBackdrop->SetWorldLocationAndRotation(EyeLocation-ToPlayer*320.f,(ToPlayer.Rotation()+FRotator(90.f,0.f,0.f)).Quaternion());
 ScareBackdrop->SetWorldScale3D(FVector(16.f,16.f,1.f));
 // Thrown from over the player's left shoulder: straight-on light flattens the head into a black mass.
 const FVector ViewRight=FVector::CrossProduct(FVector::UpVector,-ToPlayer).GetSafeNormal();
 const FVector LightSpot=EyeLocation+FVector(0,0,55)-ViewRight*45.f-ToPlayer*15.f;
 ScareLight->SetWorldLocationAndRotation(LightSpot,(HeadLocation-LightSpot).Rotation());

 // The view stays locked on the creature, with a shake that decays as the seconds run out.
 if(PC)
 {
  const float ShakeAmount=JumpscareShake*FMath::Clamp(1.f-CatchTime/FMath::Max(.2f,JumpscareSeconds),0.f,1.f);
  FRotator Look=(HeadLocation-EyeLocation).Rotation();
  Look.Pitch+=FMath::FRandRange(-ShakeAmount,ShakeAmount);
  Look.Yaw+=FMath::FRandRange(-ShakeAmount,ShakeAmount);
  Look.Roll+=FMath::FRandRange(-ShakeAmount,ShakeAmount)*1.5f;
  PC->SetControlRotation(Look);
  // One red blink on contact, before the slower fade to black.
  if(CatchTime<Dt*1.5f && PC->PlayerCameraManager)
   PC->PlayerCameraManager->StartCameraFade(.75f,0.f,.35f,FLinearColor(.6f,0.f,0.f),false,false);
 }
 const float FadeStart=FMath::Max(.2f,JumpscareSeconds-.5f);
 if(!bFadeStarted && CatchTime>=FadeStart && PC && PC->PlayerCameraManager)
 {
  PC->PlayerCameraManager->StartCameraFade(0.f,1.f,JumpscareSeconds-FadeStart,FLinearColor::Black,false,true);
  bFadeStarted=true;
 }
 if(!bRestartRequested && CatchTime>=JumpscareSeconds)
 {
  bRestartRequested=true;
  // A fresh level reload clears progress, pickups and the creature; the checkpoint option puts
  // the player back at the hospital door with a flashlight instead of out in the forest.
  UGameplayStatics::OpenLevel(this,FName(*UGameplayStatics::GetCurrentLevelName(this,true)),true,TEXT("checkpoint=entrance"));
 }
}

void AShiftMonster::Tick(float Dt)
{
 Super::Tick(Dt);
 Age+=Dt;ScreamCooldown-=Dt;FreezeTime-=Dt;GrowlCooldown-=Dt;
 APawn* Target=UGameplayStatics::GetPlayerPawn(this,0);
 if(!Target) return;
 if(bCaughtPlayer){JumpscareTick(Target,Dt);return;}
 const FVector Delta=Target->GetActorLocation()-GetActorLocation();
 const float Distance=Delta.Size2D();
 // Someone inside a hiding cabinet is out of reach, even when the creature stands right against it.
 const bool bTargetHidden=Target->ActorHasTag(TEXT("PlayerHidden"));
 const bool Visible=!bTargetHidden && Distance<DetectionDistance && FMath::Abs(Delta.Z)<450.f && CanSee(Target);
 if(Visible)
 {
  LastSeen=Target->GetActorLocation();Memory=7.f;
  if(!bChasing){bChasing=true;FreezeTime=.65f;Roar();}
 }
 else Memory-=Dt;
 // Losing sight does not end it: the last place the player stood is worth searching.
 if(Memory<=0.f&&bChasing){bChasing=false;SearchTime=SearchSeconds;}
 if(bChasing)
 {
  const FVector Desired=(LastSeen-GetActorLocation()).GetSafeNormal2D();
  if(!Desired.IsNearlyZero()) SetActorRotation(FMath::RInterpTo(GetActorRotation(),Desired.Rotation(),Dt,4.f));
  if(!bTargetHidden && Distance<CatchDistance && FMath::Abs(Delta.Z)<220.f)
  {
   CatchPlayer(Target);
   return;
  }
  if(FreezeTime<=0.f && Distance>130.f)
  {
   GetCharacterMovement()->MaxWalkSpeed=ChaseSpeed;
   AddMovementInput(Steer(Desired,Target),1.f);
  }
  else GetCharacterMovement()->StopMovementImmediately();
  if(GrowlCooldown<=0.f && !Voice->IsPlaying() && Growl)
  {Voice->SetSound(Growl);Voice->SetPitchMultiplier(FMath::FRandRange(.85f,1.f));Voice->Play();GrowlCooldown=FMath::FRandRange(5.f,8.f);}
 }
 else if(FreezeTime<=0.f) Prowl(Target,Dt);
 const float Speed=GetVelocity().Size2D();
 const bool ShouldWalk=Speed>8.f;
 if(ShouldWalk!=bWalkingAnimation)
 {
  bWalkingAnimation=ShouldWalk;
  if(UAnimSequence* Clip=ShouldWalk?WalkAnimation.Get():IdleAnimation.Get()) GetMesh()->PlayAnimation(Clip,true);
 }
 GetMesh()->SetPlayRate(ShouldWalk?FMath::Clamp(Speed/137.f,.25f,2.3f):1.f);
 GetMesh()->SetRelativeLocation(VisualOrigin);
}
