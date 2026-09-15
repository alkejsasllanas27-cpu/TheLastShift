#include "ShiftLadder.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"

namespace
{
 const FName ClimbingTag(TEXT("PlayerClimbing"));
 FCollisionQueryParams LadderQuery(const ACharacter* Character,const AActor* Ladder)
 {
  FCollisionQueryParams Params(SCENE_QUERY_STAT(ShiftLadder),false,Character);
  Params.AddIgnoredActor(Ladder);
  // Match the character's existing movement exclusions, without changing them.
  for(AActor* Actor:Character->GetCapsuleComponent()->GetMoveIgnoreActors()) Params.AddIgnoredActor(Actor);
  return Params;
 }
 FCollisionShape CharacterShape(const ACharacter* Character)
 {
  const auto* Capsule=Character->GetCapsuleComponent();
  return FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(),Capsule->GetScaledCapsuleHalfHeight());
 }
}

AShiftLadder::AShiftLadder()
{
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
 RootComponent->SetMobility(EComponentMobility::Static);
 InteractionTarget=CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTarget"));
 InteractionTarget->SetupAttachment(RootComponent);
 InteractionTarget->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
 InteractionTarget->SetCollisionResponseToAllChannels(ECR_Ignore);
 InteractionTarget->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
 InteractionTarget->SetGenerateOverlapEvents(false);
 PrimaryActorTick.bCanEverTick=false;
}

void AShiftLadder::OnConstruction(const FTransform& Transform)
{
 Super::OnConstruction(Transform);
 Rebuild();
}

UStaticMeshComponent* AShiftLadder::AddPart(FName Name,const FVector& Location,const FVector& Size)
{
 auto* Part=NewObject<UStaticMeshComponent>(this,Name);
 Part->CreationMethod=EComponentCreationMethod::UserConstructionScript;
 Part->SetupAttachment(RootComponent);
 Part->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
 Part->SetRelativeLocation(Location);
 Part->SetRelativeScale3D(Size/100.f);
 Part->SetMobility(EComponentMobility::Static);
 Part->SetCollisionProfileName(TEXT("BlockAll"));
 Part->SetGenerateOverlapEvents(false);
 if(LadderMaterial) Part->SetMaterial(0,LadderMaterial);
 else if(auto* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/HorrorDressing/M_AgedSteel.M_AgedSteel"))) Part->SetMaterial(0,Material);
 Part->RegisterComponent();
 Parts.Add(Part);
 return Part;
}

void AShiftLadder::Rebuild()
{
 if(IsClimbing()) return;
 for(auto Part:Parts) if(IsValid(Part)) Part->DestroyComponent();
 Parts.Empty();
 Height=FMath::Max(120.f,Height);
 Width=FMath::Max(35.f,Width);
 InteractionTarget->SetRelativeLocation(FVector(12,0,Height*.5f));
 InteractionTarget->SetBoxExtent(FVector(18,Width*.5f+8,Height*.5f+30));
 if(LadderMesh)
 {
  auto* Part=AddPart(TEXT("AuthoredLadder"),FVector::ZeroVector,FVector(100));
  Part->SetStaticMesh(LadderMesh);
  Part->SetRelativeTransform(MeshTransform);
  if(!LadderMaterial) Part->EmptyOverrideMaterials();
  return;
 }
 // Rails extend above the upper floor to provide handholds during dismount.
 const float RungHeight=Height+FMath::Max(0.f,TopClearance);
 const float RailHeight=RungHeight+75.f;
 AddPart(TEXT("LeftRail"),FVector(0,-Width*.5f,RailHeight*.5f),FVector(8,7,RailHeight));
 AddPart(TEXT("RightRail"),FVector(0,Width*.5f,RailHeight*.5f),FVector(8,7,RailHeight));
 const int32 Count=FMath::Max(4,FMath::CeilToInt(RungHeight/28.f));
 for(int32 Index=1;Index<=Count;++Index)
  AddPart(FName(*FString::Printf(TEXT("Rung%d"),Index)),FVector(2,0,Index*RungHeight/Count-8),FVector(11,Width,5));
 for(int32 Index=0;Index<3;++Index)
 {
  const float Z=35+(Height-70)*Index*.5f;
  AddPart(FName(*FString::Printf(TEXT("LeftMount%d"),Index)),FVector(-12,-Width*.5f,Z),FVector(24,7,7));
  AddPart(FName(*FString::Printf(TEXT("RightMount%d"),Index)),FVector(-12,Width*.5f,Z),FVector(24,7,7));
 }
}

FVector AShiftLadder::EndAnchor(ACharacter* Character,bool bTop) const
{
 const float Radius=Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
 const float HalfHeight=Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
 FVector Anchor=GetActorTransform().TransformPosition(FVector(FMath::Max(StandOff,Radius+18),0,bTop?Height+FMath::Max(0.f,TopClearance):0));
 Anchor.Z+=HalfHeight+(bTop?8.f:4.f);
 return Anchor;
}

bool AShiftLadder::CapsuleFits(ACharacter* Character,const FVector& Center) const
{
 return !GetWorld()->OverlapBlockingTestByProfile(Center,FQuat::Identity,
  Character->GetCapsuleComponent()->GetCollisionProfileName(),CharacterShape(Character),LadderQuery(Character,this));
}

bool AShiftLadder::FindLanding(ACharacter* Character,bool bTop,FVector& OutCenter) const
{
 FVector Local=bTop?TopExitOffset+FVector(0,0,Height):BottomExitOffset;
 const FVector Floor=GetActorTransform().TransformPosition(Local);
 const auto* Movement=Character->GetCharacterMovement();
 const float Step=FMath::Max(Movement->MaxStepHeight,MaxLandingAdjustment);
 FHitResult Hit;
 if(!GetWorld()->LineTraceSingleByProfile(Hit,Floor+FVector(0,0,Step+12),Floor-FVector(0,0,Step+20),
  Character->GetCapsuleComponent()->GetCollisionProfileName(),LadderQuery(Character,this)) || !Movement->IsWalkable(Hit)) return false;
 OutCenter=Hit.ImpactPoint+FVector(0,0,Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+2);
 if(!CapsuleFits(Character,OutCenter)) return false;
 // Require support underneath most of the capsule; a thin rail or ledge is not a landing.
 const float SupportRadius=Character->GetCapsuleComponent()->GetScaledCapsuleRadius()*.7f;
 for(const FVector Offset:{FVector(SupportRadius,0,0),FVector(-SupportRadius,0,0),FVector(0,SupportRadius,0),FVector(0,-SupportRadius,0)})
 {
  FHitResult Support;
  if(!GetWorld()->LineTraceSingleByProfile(Support,Hit.ImpactPoint+Offset+FVector(0,0,8),Hit.ImpactPoint+Offset-FVector(0,0,12),
   Character->GetCapsuleComponent()->GetCollisionProfileName(),LadderQuery(Character,this)) || !Movement->IsWalkable(Support)) return false;
 }
 return true;
}

bool AShiftLadder::ClearPath(ACharacter* Character,const TArray<FVector>& Points) const
{
 FVector Start=Character->GetActorLocation();
 for(const FVector& End:Points)
 {
  FHitResult Hit;
  if(!CapsuleFits(Character,End) || GetWorld()->SweepSingleByProfile(Hit,Start,End,FQuat::Identity,
   Character->GetCapsuleComponent()->GetCollisionProfileName(),CharacterShape(Character),LadderQuery(Character,this))) return false;
  Start=End;
 }
 return true;
}

bool AShiftLadder::CanInteract(ACharacter* Character) const
{
 if(!IsValid(Character)) return false;
 if(IsClimbing()) return Climber.Get()==Character;
 // Capsules stay upright; tilted ladders need their own animation/movement contract.
 if(FVector::DotProduct(GetActorUpVector(),FVector::UpVector)<.99f || !Character->GetActorEnableCollision()) return false;
 const auto* Movement=Character->GetCharacterMovement();
 if(!Movement || !Movement->IsMovingOnGround() || !Character->GetCapsuleComponent()->IsQueryCollisionEnabled()) return false;
 for(bool bTop:{false,true})
 {
  FVector Landing;
  if(!FindLanding(Character,bTop,Landing)) continue;
  const FVector Delta=Character->GetActorLocation()-Landing;
  if(Delta.Size2D()>EntryReach || FMath::Abs(Delta.Z)>Movement->MaxStepHeight+18) continue;
  const FVector Anchor=EndAnchor(Character,bTop);
  const float SafeZ=FMath::Max(Anchor.Z,Character->GetActorLocation().Z);
  const TArray<FVector> Entry={FVector(Character->GetActorLocation().X,Character->GetActorLocation().Y,SafeZ),FVector(Anchor.X,Anchor.Y,SafeZ),Anchor};
  if(ClearPath(Character,Entry)) return true;
 }
 return false;
}

FText AShiftLadder::GetPrompt() const
{
 return FText::FromString(IsClimbing()?TEXT("W / S - climb up / down  |  E - step off at either end"):TEXT("E - Climb ladder"));
}

bool AShiftLadder::Interact(ACharacter* Character,APlayerController* Controller)
{
 if(LastInteractionFrame==GFrameCounter) return false;
 LastInteractionFrame=GFrameCounter;
 LastFailureReason=FText::GetEmpty();
 if(IsClimbing()) return Climber.Get()==Character && Cancel();
 if(!IsValid(Controller) || Controller->GetPawn()!=Character || !CanInteract(Character))
 {
  LastFailureReason=FText::FromString(TEXT("Approach either end of the ladder. The landing must be clear."));
  return false;
 }
 float BestDistance=TNumericLimits<float>::Max();
 TArray<FVector> EntryRoute;
 for(bool bTop:{false,true})
 {
  FVector Landing;
  if(!FindLanding(Character,bTop,Landing)) continue;
  const FVector Delta=Character->GetActorLocation()-Landing;
  if(Delta.Size2D()>EntryReach || FMath::Abs(Delta.Z)>Character->GetCharacterMovement()->MaxStepHeight+18) continue;
  const FVector Anchor=EndAnchor(Character,bTop);
  const float SafeZ=FMath::Max(Anchor.Z,Character->GetActorLocation().Z);
  TArray<FVector> Candidate={FVector(Character->GetActorLocation().X,Character->GetActorLocation().Y,SafeZ),FVector(Anchor.X,Anchor.Y,SafeZ),Anchor};
  if(Delta.SizeSquared()<BestDistance && ClearPath(Character,Candidate))
  {
   BestDistance=Delta.SizeSquared(); EntryRoute=MoveTemp(Candidate);
  }
 }
 if(EntryRoute.IsEmpty()) return false;
 Climber=Character;
 ClimbController=Controller;
 auto* Movement=Character->GetCharacterMovement();
 SavedMovementMode=Movement->MovementMode;
 SavedCustomMovementMode=Movement->CustomMovementMode;
 bAddedActorIgnore=!Character->GetCapsuleComponent()->GetMoveIgnoreActors().Contains(this);
 if(bAddedActorIgnore) Character->MoveIgnoreActorAdd(this);
 Controller->SetIgnoreMoveInput(true); bAddedInputIgnore=true;
 bAddedClimbingTag=!Character->Tags.Contains(ClimbingTag);
 if(bAddedClimbingTag) Character->Tags.Add(ClimbingTag);
 Character->ConsumeMovementInputVector();
 Movement->StopMovementImmediately();
 Movement->DisableMovement();
 LowerZ=EndAnchor(Character,false).Z;
 UpperZ=EndAnchor(Character,true).Z;
 Route=MoveTemp(EntryRoute); RouteIndex=0; Traversal=ETraversal::Aligning;
 return true;
}

bool AShiftLadder::MoveSwept(const FVector& Target)
{
 auto* Character=Climber.Get();
 if(!Character) return false;
 FHitResult Hit;
 Character->SetActorLocation(Target,true,&Hit,ETeleportType::None);
 if(Hit.bBlockingHit && !Character->GetActorLocation().Equals(Target,.5f))
 {
  LastFailureReason=FText::FromString(TEXT("The ladder path is blocked."));
  return false;
 }
 return true;
}

void AShiftLadder::AdvanceRoute(float Distance)
{
 while(Climber.IsValid() && RouteIndex<Route.Num() && Distance>0)
 {
  const FVector Current=Climber->GetActorLocation();
  const FVector Delta=Route[RouteIndex]-Current;
  const float Remaining=Delta.Size();
  if(Remaining<.5f) { ++RouteIndex; continue; }
  const float Step=FMath::Min(Distance,Remaining);
  if(!MoveSwept(Current+Delta*(Step/Remaining))) return;
  Distance-=Step;
  if(Step>=Remaining-.01f) ++RouteIndex;
 }
 if(RouteIndex>=Route.Num())
 {
  Route.Empty();
  if(Traversal==ETraversal::Dismounting) RestorePlayer();
  else Traversal=ETraversal::Climbing;
 }
}

bool AShiftLadder::BeginDismount(bool bTop)
{
 auto* Character=Climber.Get();
 if(!Character) return false;
 FVector Landing;
 if(!FindLanding(Character,bTop,Landing))
 {
  LastFailureReason=FText::FromString(TEXT("The landing is blocked or has no safe floor.")); return false;
 }
 const FVector Current=Character->GetActorLocation();
 const float SafeZ=FMath::Max(Current.Z,Landing.Z+6);
 TArray<FVector> ExitRoute={FVector(Current.X,Current.Y,SafeZ),FVector(Landing.X,Landing.Y,SafeZ),Landing};
 if(!ClearPath(Character,ExitRoute))
 {
  LastFailureReason=FText::FromString(TEXT("The exit is blocked. Use the other end of the ladder.")); return false;
 }
 Route=MoveTemp(ExitRoute); RouteIndex=0; Traversal=ETraversal::Dismounting;
 return true;
}

bool AShiftLadder::Cancel()
{
 if(!Climber.IsValid()) return false;
 if(Traversal!=ETraversal::Climbing) return false;
 const float Z=Climber->GetActorLocation().Z;
 if(FMath::Abs(Z-LowerZ)<12) return BeginDismount(false);
 if(FMath::Abs(Z-UpperZ)<12) return BeginDismount(true);
 LastFailureReason=FText::FromString(TEXT("Use W or S to reach the top or bottom before stepping off."));
 return false;
}

void AShiftLadder::ClimbAxis(float Axis,float DeltaSeconds)
{
 if(!IsClimbing())
 {
  if(Traversal!=ETraversal::None) RestorePlayer();
  return;
 }
 if(!ClimbController.IsValid() || ClimbController->GetPawn()!=Climber.Get()) { RestorePlayer(); return; }
 Climber->ConsumeMovementInputVector();
 Axis=FMath::Clamp(Axis,-1.f,1.f);
 float Time=FMath::Clamp(DeltaSeconds,0.f,.5f);
 // Substeps prevent large low-frame-rate jumps; every segment sweeps the existing capsule.
 while(Time>UE_SMALL_NUMBER && IsClimbing())
 {
  const float Step=FMath::Min(Time,.04f); Time-=Step;
  if(Traversal==ETraversal::Aligning || Traversal==ETraversal::Dismounting)
  {
   AdvanceRoute(FMath::Max(30.f,ClimbSpeed)*Step); continue;
  }
  if(FMath::Abs(Axis)<.05f) continue;
  const FVector Current=Climber->GetActorLocation();
  FVector Target=Current;
  Target.Z=FMath::Clamp(Current.Z+Axis*FMath::Max(30.f,ClimbSpeed)*Step,LowerZ,UpperZ);
  if(!MoveSwept(Target)) return;
  if((Axis>0 && Target.Z>=UpperZ-.1f) || (Axis<0 && Target.Z<=LowerZ+.1f))
  {
   if(!BeginDismount(Axis>0)) return;
  }
 }
}

void AShiftLadder::RestorePlayer()
{
 if(auto* Character=Climber.Get())
 {
  if(bAddedActorIgnore) Character->MoveIgnoreActorRemove(this);
  if(bAddedClimbingTag) Character->Tags.Remove(ClimbingTag);
  Character->ConsumeMovementInputVector();
  if(auto* Movement=Character->GetCharacterMovement())
  {
   Movement->StopMovementImmediately();
   Movement->SetMovementMode(static_cast<EMovementMode>(SavedMovementMode),SavedCustomMovementMode);
  }
 }
 if(bAddedInputIgnore && ClimbController.IsValid()) ClimbController->SetIgnoreMoveInput(false);
 bAddedInputIgnore=false; bAddedActorIgnore=false; bAddedClimbingTag=false;
 Climber.Reset(); ClimbController.Reset(); Route.Empty(); RouteIndex=0; Traversal=ETraversal::None;
}

void AShiftLadder::EndPlay(const EEndPlayReason::Type Reason)
{
 RestorePlayer();
 Super::EndPlay(Reason);
}
