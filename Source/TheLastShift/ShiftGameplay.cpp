#include "ShiftGameplay.h"
#include "FlashlightComponent.h"
#include "ShiftLadder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/InputComponent.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "UObject/UnrealType.h"
#include "Sound/SoundBase.h"
#include "Materials/MaterialInterface.h"
#include "Styling/CoreStyle.h"
#include "Camera/PlayerCameraManager.h"
#include "InputCoreTypes.h"

namespace {
 bool BoolValue(UObject* O,const TCHAR* Name){auto* P=O?FindFProperty<FBoolProperty>(O->GetClass(),Name):nullptr;return P&&P->GetPropertyValue_InContainer(O);}
 void SetBool(UObject* O,const TCHAR* Name,bool V){if(auto* P=O?FindFProperty<FBoolProperty>(O->GetClass(),Name):nullptr)P->SetPropertyValue_InContainer(O,V);}
 AActor* ActorValue(UObject* O,const TCHAR* Name){auto* P=O?FindFProperty<FObjectProperty>(O->GetClass(),Name):nullptr;return P?Cast<AActor>(P->GetObjectPropertyValue_InContainer(O)):nullptr;}
 void Sound(AActor* A,USoundBase* S,float Volume=.45f){if(S)UGameplayStatics::PlaySoundAtLocation(A,S,A->GetActorLocation(),Volume);}
}

AShiftInteractable::AShiftInteractable()
{
 RootComponent=CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
 MovingRoot=CreateDefaultSubobject<USceneComponent>(TEXT("MovingPart"));MovingRoot->SetupAttachment(RootComponent);
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.bStartWithTickEnabled=false;
}
UStaticMeshComponent* AShiftInteractable::Part(FName Name,FVector Location,FVector Size,USceneComponent* Parent,bool Metal)
{
 auto* C=NewObject<UStaticMeshComponent>(this,Name);C->CreationMethod=EComponentCreationMethod::UserConstructionScript;
 C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
 C->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,Kind==EShiftObjectKind::Note?TEXT("/Game/ShiftAssets/Drawer/M_NotePaper.M_NotePaper"):Metal?TEXT("/Game/HorrorDressing/M_AgedSteel.M_AgedSteel"):TEXT("/Game/HospitalExterior2/92m_78.92m_78")));
 C->SetupAttachment(Parent);C->SetRelativeLocation(Location);C->SetRelativeScale3D(Size/100.f);
 C->SetCollisionProfileName(TEXT("BlockAll"));C->SetMobility(EComponentMobility::Movable);C->RegisterComponent();Parts.Add(C);return C;
}
void AShiftInteractable::OnConstruction(const FTransform& T){Super::OnConstruction(T);Rebuild();}
void AShiftInteractable::Rebuild()
{
 for(auto P:Parts)if(IsValid(P))P->DestroyComponent();Parts.Empty();
 MovingRoot->SetRelativeTransform(FTransform::Identity);
 if(Kind==EShiftObjectKind::Battery){
  auto* C=Part(TEXT("BatteryBody"),FVector(0,0,4),FVector(4,4,8),RootComponent,true);
  C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
  Part(TEXT("BatteryCap"),FVector(0,0,8.3),FVector(2.4,2.4,.6),RootComponent,true);
 }else if(Kind==EShiftObjectKind::Note){
  Part(TEXT("Paper"),FVector(0,0,.2),FVector(22,16,.4),RootComponent,false);
 }else if(Kind==EShiftObjectKind::Switch){
  Part(TEXT("Plate"),FVector(0,0,0),FVector(3,9,13),RootComponent,true);
 }else if(Kind==EShiftObjectKind::Drawer){
  if(DrawerBody && DrawerMeshes.Num()==6){
   auto MeshPart=[&](FName Name,UStaticMesh* Mesh,USceneComponent* Parent){auto* C=Part(Name,FVector::ZeroVector,FVector(100),Parent);C->SetStaticMesh(Mesh);C->EmptyOverrideMaterials();C->SetRelativeRotation(FRotator(0,-90,0));};
   MeshPart(TEXT("CabinetBody"),DrawerBody,RootComponent);
   for(int32 i=0;i<6;i++)MeshPart(FName(*FString::Printf(TEXT("AuthenticDrawer%d"),i)),DrawerMeshes[i],i==1?MovingRoot.Get():RootComponent.Get());
   ApplyPose();return;
  }
  Part(TEXT("LeftWall"),FVector(0,-43,28),FVector(56,3,56),RootComponent);
  Part(TEXT("RightWall"),FVector(0,43,28),FVector(56,3,56),RootComponent);
  Part(TEXT("Back"),FVector(-27,0,28),FVector(3,86,56),RootComponent);
  Part(TEXT("Top"),FVector(0,0,57),FVector(60,92,4),RootComponent);
  Part(TEXT("Base"),FVector(0,0,3),FVector(56,86,6),RootComponent);
  Part(TEXT("Tray"),FVector(0,0,23),FVector(48,80,2),MovingRoot);
  Part(TEXT("Front"),FVector(27,0,36),FVector(3,86,28),MovingRoot);
  Part(TEXT("TrayLeft"),FVector(0,-39,32),FVector(48,2,18),MovingRoot);
  Part(TEXT("TrayRight"),FVector(0,39,32),FVector(48,2,18),MovingRoot);
  Part(TEXT("TrayBack"),FVector(-23,0,32),FVector(2,80,18),MovingRoot);
  Part(TEXT("Handle"),FVector(30,0,39),FVector(3,20,2),MovingRoot,true);
 }else{
  Part(TEXT("LeftWall"),FVector(0,-55,105),FVector(90,4,210),RootComponent,true);
  Part(TEXT("RightWall"),FVector(0,55,105),FVector(90,4,210),RootComponent,true);
  Part(TEXT("Back"),FVector(-45,0,105),FVector(4,110,210),RootComponent,true);
  Part(TEXT("Top"),FVector(0,0,210),FVector(94,114,4),RootComponent,true);
  Part(TEXT("Base"),FVector(0,0,2),FVector(90,110,4),RootComponent,true);
  MovingRoot->SetRelativeLocation(FVector(46,-53,0));
  Part(TEXT("Door"),FVector(0,53,106),FVector(3,106,206),MovingRoot,true);
  Part(TEXT("Handle"),FVector(3,94,104),FVector(4,3,20),MovingRoot,true);
  // Raised vent detailing on the cabinet door.
  for(int i=0;i<4;i++)Part(FName(*FString::Printf(TEXT("Vent%d"),i)),FVector(2,53,155+i*6),FVector(2,52,2),MovingRoot,true);
 }
 ApplyPose();
}
void AShiftInteractable::ApplyPose()
{
 float A=OpenAlpha*OpenAlpha*(3-2*OpenAlpha);
 if(Kind==EShiftObjectKind::Drawer)MovingRoot->SetRelativeLocation(FVector(38*A,0,0));
 else if(Kind==EShiftObjectKind::Cabinet||Kind==EShiftObjectKind::HidingCabinet)MovingRoot->SetRelativeRotation(FRotator(0,-100*A,0));
}
void AShiftInteractable::SetOpen(bool V)
{
 if(bOpen==V)return;bOpen=V;Sound(this,V?OpenSound:CloseSound);SetActorTickEnabled(true);
}
void AShiftInteractable::Tick(float Dt)
{
 Super::Tick(Dt);float Target=bOpen?1.f:0.f;
 OpenAlpha=FMath::FInterpConstantTo(OpenAlpha,Target,Dt,1.25f);ApplyPose();
 if(FMath::IsNearlyEqual(OpenAlpha,Target))SetActorTickEnabled(false);
}
bool AShiftInteractable::IsAvailable()const{return !bOccupied&&(!Container||Container->OpenAlpha>.9f);}
FText AShiftInteractable::GetPrompt()const
{
 switch(Kind){case EShiftObjectKind::Battery:return FText::FromString(TEXT("[E] Pick up battery"));case EShiftObjectKind::Note:return FText::FromString(TEXT("[E] Read"));case EShiftObjectKind::HidingCabinet:return FText::FromString(TEXT("[E] Hide"));case EShiftObjectKind::Switch:return FText::FromString(TEXT("[E] Use"));default:return FText::FromString(bOpen?TEXT("[E] Close"):TEXT("[E] Open"));}
}
void AShiftInteractable::Interact(AShiftDirector* D)
{
 if(!D||!IsAvailable())return;
 switch(Kind){
 case EShiftObjectKind::Battery:if(D->SpareBatteries>=3){D->Notify(TEXT("Battery pouch full"));return;}D->AddBattery();Sound(this,UseSound);Destroy();break;
 case EShiftObjectKind::Note:D->ReadNote(NoteText,StoryEvent);Sound(this,UseSound);break;
 case EShiftObjectKind::HidingCabinet:D->EnterHiding(this);break;
 case EShiftObjectKind::Switch:D->TriggerStory(StoryEvent);Sound(this,UseSound);D->Notify(TEXT("Power relay engaged"));break;
 default:if(!bOpen)D->TriggerStory(StoryEvent);SetOpen(!bOpen);break;
 }
}

void UShiftPromptWidget::NativeOnInitialized()
{
 Super::NativeOnInitialized();auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
 auto Add=[&](UTextBlock*& Text,FVector2D Pos,FVector2D Size,int32 Font){Text=WidgetTree->ConstructWidget<UTextBlock>();Text->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle("Regular",Font)));Text->SetColorAndOpacity(FSlateColor(FLinearColor(.85f,.85f,.80f)));Text->SetShadowColorAndOpacity(FLinearColor(0,0,0,.9f));Text->SetShadowOffset(FVector2D(1,1));Text->SetJustification(ETextJustify::Center);Text->SetAutoWrapText(true);auto* Slot=Canvas->AddChildToCanvas(Text);Slot->SetAnchors(FAnchors(.5f,.5f));Slot->SetAlignment(FVector2D(.5f,0));Slot->SetPosition(Pos);Slot->SetSize(Size);};
 UTextBlock* T=nullptr;Add(T,FVector2D(0,95),FVector2D(450,45),16);PromptText=T;
 Add(T,FVector2D(0,140),FVector2D(550,65),15);StatusText=T;
 Add(T,FVector2D(0,-200),FVector2D(560,280),21);NoteBody=T;
 Add(T,FVector2D(36,36),FVector2D(450,65),15);ObjectiveText=T;
 auto* ObjectiveSlot=Cast<UCanvasPanelSlot>(T->Slot);ObjectiveSlot->SetAnchors(FAnchors(0,0));ObjectiveSlot->SetAlignment(FVector2D(0,0));T->SetJustification(ETextJustify::Left);
 Add(T,FVector2D(-36,-80),FVector2D(340,34),16);BatteryText=T;
 auto* BatterySlot=Cast<UCanvasPanelSlot>(T->Slot);BatterySlot->SetAnchors(FAnchors(1,1));BatterySlot->SetAlignment(FVector2D(1,1));T->SetJustification(ETextJustify::Right);
 BatteryBar=WidgetTree->ConstructWidget<UProgressBar>();auto* BarSlot=Canvas->AddChildToCanvas(BatteryBar);BarSlot->SetAnchors(FAnchors(1,1));BarSlot->SetAlignment(FVector2D(1,1));BarSlot->SetPosition(FVector2D(-36,-44));BarSlot->SetSize(FVector2D(170,5));
 SetVisibility(ESlateVisibility::HitTestInvisible);
}
void UShiftPromptWidget::Show(const FString& P,const FString& S,const FString& N)
{if(PromptText)PromptText->SetText(FText::FromString(P));if(StatusText)StatusText->SetText(FText::FromString(S));if(NoteBody)NoteBody->SetText(FText::FromString(N));}
void UShiftPromptWidget::UpdateBattery(float Charge,int32 Spares,bool On)
{
 if(!BatteryText)return;Charge=FMath::Clamp(Charge,0.f,1.f);
 BatteryText->SetText(FText::FromString(FString::Printf(TEXT("FLASHLIGHT  %d%%  |  %s\nSPARES  %d / 3  ·  R REPLACE"),FMath::CeilToInt(Charge*100),On?TEXT("ON"):TEXT("OFF"),Spares)));
 FLinearColor Color=Charge<=.15f?FLinearColor(.9f,.25f,.12f):FLinearColor(.68f,.77f,.67f);BatteryBar->SetPercent(Charge);BatteryBar->SetFillColorAndOpacity(Color);
}
void UShiftPromptWidget::UpdateObjective(const FString& Objective){if(ObjectiveText)ObjectiveText->SetText(FText::FromString(Objective));}

AShiftDirector::AShiftDirector(){PrimaryActorTick.bCanEverTick=true;SetActorHiddenInGame(true);}
bool AShiftDirector::BindPlayer()
{
 PC=UGameplayStatics::GetPlayerController(this,0);Player=PC?Cast<ACharacter>(PC->GetPawn()):nullptr;
 if(!Player||!PC->IsLocalController())return false;
 Flashlight=Player->FindComponentByClass<UFlashlightComponent>();
 Widget=CreateWidget<UShiftPromptWidget>(PC,UShiftPromptWidget::StaticClass());if(Widget)Widget->AddToViewport(20);
 // The two existing entrance controllers retain their animation graphs.
 for(TActorIterator<AActor> It(GetWorld());It;++It){AActor* A=*It;if(ActorValue(A,TEXT("LeftLeaf"))&&ActorValue(A,TEXT("RightLeaf"))){DoorControllers.Add(ActorValue(A,TEXT("LeftLeaf")),A);DoorControllers.Add(ActorValue(A,TEXT("RightLeaf")),A);}}
 // A narrow query surface lets the camera target barred leaves through their gaps.
 for(const auto& Entry:DoorControllers){AActor* Leaf=Entry.Key;FBox B=Leaf->CalculateComponentsBoundingBoxInLocalSpace();auto* Target=NewObject<UBoxComponent>(Leaf);Target->SetupAttachment(Leaf->GetRootComponent());Target->SetRelativeLocation(B.GetCenter());Target->SetBoxExtent(B.GetExtent().ComponentMax(FVector(3)));Target->SetCollisionEnabled(ECollisionEnabled::QueryOnly);Target->SetCollisionResponseToAllChannels(ECR_Ignore);Target->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);Target->RegisterComponent();}
 if(GetWorld()->WorldType==EWorldType::Game)ToggleMenu();
 return true;
}
void AShiftDirector::UpdateFocus()
{
 FocusedActor=nullptr;if(!Player||bPlayerHidden||ActiveLadder||!Reading.IsEmpty())return;
 FVector Loc;FRotator Rot;PC->GetPlayerViewPoint(Loc,Rot);FHitResult Hit;
 FCollisionQueryParams Params(SCENE_QUERY_STAT(ShiftInteract),true,Player);
 if(GetWorld()->LineTraceSingleByChannel(Hit,Loc,Loc+Rot.Vector()*240.f,ECC_Visibility,Params)){
  AActor* A=Hit.GetActor();if(auto* I=Cast<AShiftInteractable>(A)){if(I->IsAvailable())FocusedActor=A;}
  else if(auto* Ladder=Cast<AShiftLadder>(A)){if(Ladder->CanInteract(Player))FocusedActor=Ladder;}
  else if(DoorControllers.Contains(A)|| (A&&A->FindFunction(TEXT("Interact"))))FocusedActor=A;
 }
 // Small pickups get forgiving aim without extending reach or ignoring occlusion.
 // A deliberate hit on another interaction (for example a note) keeps priority.
 auto* DirectObject=Cast<AShiftInteractable>(FocusedActor);
 if(FocusedActor&&(!DirectObject||(DirectObject->Kind!=EShiftObjectKind::Drawer&&DirectObject->Kind!=EShiftObjectKind::Cabinet)))return;
 if(SpareBatteries>=3)return;
 const FVector Forward=Rot.Vector();float BestScore=TNumericLimits<float>::Max();
 for(TActorIterator<AShiftInteractable> It(GetWorld());It;++It){
  AShiftInteractable* Battery=*It;
  if(Battery->Kind!=EShiftObjectKind::Battery||!Battery->IsAvailable())continue;
  if(DirectObject&&Battery->Container!=DirectObject)continue;
  FVector Center,Extent;Battery->GetActorBounds(true,Center,Extent);
  const FVector ToBattery=Center-Loc;const float Distance=ToBattery.Size();
  const float Along=FVector::DotProduct(ToBattery,Forward);
  const float Across=(ToBattery-Forward*Along).Size();
  if(Distance>240.f||Along<=0||Across>32.f||Along<FMath::Cos(FMath::DegreesToRadians(14.f))*Distance)continue;
  bool bVisible=false;
  for(const FVector& Point:{Center,Center+FVector(0,0,Extent.Z*.75f)}){
   FHitResult Occluder;
   if(!GetWorld()->LineTraceSingleByChannel(Occluder,Loc,Point,ECC_Visibility,Params)||Occluder.GetActor()==Battery){bVisible=true;break;}
  }
  if(!bVisible)continue;
  const float Score=Across/FMath::Max(1.f,Along)+Distance*.0001f;
  if(Score<BestScore){BestScore=Score;FocusedActor=Battery;}
 }
}
FString AShiftDirector::Prompt()const
{
 if(bPlayerHidden)return TEXT("[E] Exit");if(!Reading.IsEmpty())return TEXT("[E] Put away");
 if(IsValid(ActiveLadder))return ActiveLadder->GetPrompt().ToString();
 if(auto* Ladder=Cast<AShiftLadder>(FocusedActor))return Ladder->GetPrompt().ToString();
 if(auto* I=Cast<AShiftInteractable>(FocusedActor))return I->GetPrompt().ToString();
 if(FocusedActor){if(const auto* C=DoorControllers.Find(FocusedActor))return BoolValue(*C,TEXT("IsOpen"))?TEXT("[E] Close"):TEXT("[E] Open");return BoolValue(FocusedActor,TEXT("bIsOpen"))?TEXT("[E] Close"):TEXT("[E] Open");}
 return TEXT("");
}
void AShiftDirector::Tick(float Dt)
{
 Super::Tick(Dt);if(!IsValid(Player)){if(!BindPlayer())return;}
 if(!IsValid(Flashlight))Flashlight=Player->FindComponentByClass<UFlashlightComponent>();
 if(PC->WasInputKeyJustPressed(EKeys::P)||(GetWorld()->WorldType!=EWorldType::PIE&&PC->WasInputKeyJustPressed(EKeys::Escape)))ToggleMenu();
 if(bMenuOpen)return;
 if(IsValid(ActiveLadder)){
  ActiveLadder->ClimbAxis((PC->IsInputKeyDown(EKeys::W)?1.f:0.f)-(PC->IsInputKeyDown(EKeys::S)?1.f:0.f),Dt);
  if(!ActiveLadder->IsClimbing())ActiveLadder=nullptr;
 }
 if(bPlayerHidden){FRotator R=PC->GetControlRotation();R.Yaw=HideFacing.Yaw+FMath::Clamp(FMath::FindDeltaAngleDegrees(HideFacing.Yaw,R.Yaw),-40.f,40.f);R.Pitch=FMath::Clamp(FRotator::NormalizeAxis(R.Pitch),-30.f,30.f);PC->SetControlRotation(R);if(Flashlight&&Flashlight->IsFlashlightOn())Flashlight->SetFlashlightOn(false);}
 TraceClock+=Dt;if(TraceClock>=.1f){TraceClock=0;UpdateFocus();}
 if(PC->WasInputKeyJustPressed(EKeys::E))Interact();
 if(PC->WasInputKeyJustPressed(EKeys::R)&&!bPlayerHidden&&!ActiveLadder&&Reading.IsEmpty())ReloadBattery();
 if(Flashlight){bool Low=Flashlight->BatteryCharge<=.15f;if(Low&&!bWasLow)Notify(TEXT("Flashlight battery low"),5);bWasLow=Low;}
 if(Widget)Widget->Show(Prompt(),GetWorld()->GetTimeSeconds()<StatusUntil?Status:TEXT(""),Reading);
 if(Widget&&Flashlight)Widget->UpdateBattery(Flashlight->BatteryCharge,SpareBatteries,Flashlight->IsFlashlightOn());
 if(ObjectiveStage==2&&Player->GetActorLocation().X<-13300&&FMath::Abs(Player->GetActorLocation().Y-2242)<500){ObjectiveStage=3;ReadNote(FText::FromString(TEXT("SHIFT COMPLETE\n\nYou recovered the maintenance record and left the hospital.\n\nThe power failure was no accident.\n\n[E] Return to exploration")),NAME_None);}
 if(Widget){const TCHAR* Objectives[]={TEXT("NIGHT SHIFT\nFind the log at reception."),TEXT("MAINTENANCE RECORD\nSearch the storage cabinet in the first room."),TEXT("LEAVE THE HOSPITAL\nReturn outside through the main gate."),TEXT("SHIFT COMPLETE\nMaintenance record recovered.")};Widget->UpdateObjective(Objectives[FMath::Clamp(ObjectiveStage,0,3)]);}
}
void AShiftDirector::Interact()
{
 if(bMenuOpen)return;
 if(IsValid(ActiveLadder)){
  if(!ActiveLadder->Interact(Player,PC))Notify(ActiveLadder->LastFailureReason.ToString());
  return;
 }
 if(bPlayerHidden){ExitHiding();return;}if(!Reading.IsEmpty()){Reading.Empty();return;}
 UpdateFocus();if(auto* I=Cast<AShiftInteractable>(FocusedActor)){I->Interact(this);return;}
 if(auto* Ladder=Cast<AShiftLadder>(FocusedActor)){
  if(Ladder->Interact(Player,PC))ActiveLadder=Ladder;else Notify(Ladder->LastFailureReason.ToString());
  return;
 }
 if(FocusedActor){if(auto* C=DoorControllers.Find(FocusedActor)){SetBool(*C,TEXT("IsOpen"),!BoolValue(*C,TEXT("IsOpen")));return;}if(auto* F=FocusedActor->FindFunction(TEXT("Interact")))FocusedActor->ProcessEvent(F,nullptr);return;}
}
void AShiftDirector::Notify(const FString& Text,float Seconds){Status=Text;StatusUntil=GetWorld()->GetTimeSeconds()+Seconds;}
void AShiftDirector::AddBattery(){SpareBatteries=FMath::Min(3,SpareBatteries+1);Notify(FString::Printf(TEXT("Battery collected  |  Spares: %d  |  R to replace"),SpareBatteries));}
void AShiftDirector::ReloadBattery(){if(!Flashlight)return;if(SpareBatteries<1){Notify(TEXT("No spare batteries"));return;}if(Flashlight->ReplaceBattery()){--SpareBatteries;Notify(FString::Printf(TEXT("Battery replaced  |  Spares: %d"),SpareBatteries));}else Notify(TEXT("Battery is already full"));}
void AShiftDirector::ReadNote(const FText& Text,FName Event){Reading=Text.ToString();TriggerStory(Event);}
void AShiftDirector::EnterHiding(AShiftInteractable* Cabinet)
{
 if(!Player||!Cabinet||Cabinet->bOccupied)return;
 HidingCabinet=Cabinet;Cabinet->bOccupied=true;bPlayerHidden=true;
 SavedMovementMode=Player->GetCharacterMovement()->MovementMode;bSavedCollision=Player->GetActorEnableCollision();
 Player->GetCharacterMovement()->StopMovementImmediately();Player->GetCharacterMovement()->DisableMovement();Player->SetActorEnableCollision(false);
 Player->SetActorLocation(Cabinet->GetActorTransform().TransformPosition(FVector(-5,0,100)),false,nullptr,ETeleportType::TeleportPhysics);
 HideFacing=Cabinet->GetActorRotation();PC->SetControlRotation(HideFacing);Player->Tags.AddUnique(TEXT("PlayerHidden"));
 if(Flashlight)Flashlight->SetFlashlightOn(false);Cabinet->OpenAlpha=1.f;Cabinet->bOpen=true;Cabinet->SetOpen(false);
 if(PC->PlayerCameraManager)PC->PlayerCameraManager->StartCameraFade(1,0,.35f,FLinearColor::Black);
}
void AShiftDirector::ExitHiding()
{
 if(!Player||!HidingCabinet)return;
 FVector Exit=HidingCabinet->GetActorTransform().TransformPosition(FVector(165,0,100));
 auto* Capsule=Player->GetCapsuleComponent();FCollisionQueryParams Q(SCENE_QUERY_STAT(ShiftHideExit),false,Player);Q.AddIgnoredActor(HidingCabinet);
 if(GetWorld()->OverlapBlockingTestByChannel(Exit,FQuat::Identity,ECC_Pawn,FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(),Capsule->GetScaledCapsuleHalfHeight()),Q)){Notify(TEXT("Exit blocked"));return;}
 Player->SetActorLocation(Exit,false,nullptr,ETeleportType::TeleportPhysics);Player->SetActorEnableCollision(bSavedCollision);
 Player->GetCharacterMovement()->SetMovementMode(static_cast<EMovementMode>(SavedMovementMode));Player->Tags.Remove(TEXT("PlayerHidden"));
 HidingCabinet->bOccupied=false;HidingCabinet->SetOpen(true);HidingCabinet=nullptr;bPlayerHidden=false;
}
void AShiftDirector::TriggerStory(FName Event){if(Event.IsNone())return;if(Event==TEXT("NightShiftEvidence")&&ObjectiveStage==0){ObjectiveStage=1;Notify(TEXT("Objective updated: find the maintenance record"),5);}if(Event==TEXT("MaintenanceEvidence")){if(ObjectiveStage<1){Notify(TEXT("Read the reception log first"));return;}if(ObjectiveStage==1){ObjectiveStage=2;ReadNote(FText::FromString(TEXT("MAINTENANCE RECORD\n\n03:21 — The backup feed was disconnected manually.\nThe night staff must evacuate. Take this record outside.\n\n[E] Put away")),NAME_None);}}for(TActorIterator<AShiftAudioZone> It(GetWorld());It;++It)if(It->StoryEvent==Event)It->PlayStory(true);}
void AShiftDirector::ToggleMenu()
{
 if(!PC)return;bMenuOpen=!bMenuOpen;
 if(bMenuOpen){Menu=CreateWidget<UShiftMenuWidget>(PC);Menu->Director=this;Menu->AddToViewport(100);PC->SetPause(true);PC->bShowMouseCursor=true;FInputModeUIOnly Input;Input.SetWidgetToFocus(Menu->TakeWidget());PC->SetInputMode(Input);}
 else{PC->SetPause(false);PC->bShowMouseCursor=false;PC->SetInputMode(FInputModeGameOnly());if(Menu)Menu->RemoveFromParent();Menu=nullptr;}
}
void AShiftDirector::EndPlay(const EEndPlayReason::Type Reason){if(Menu)Menu->RemoveFromParent();if(Widget)Widget->RemoveFromParent();Super::EndPlay(Reason);}
bool UShiftGameplaySubsystem::DoesSupportWorldType(EWorldType::Type T)const{return T==EWorldType::Game||T==EWorldType::PIE;}
void UShiftGameplaySubsystem::OnWorldBeginPlay(UWorld& W){Super::OnWorldBeginPlay(W);if(W.GetNetMode()!=NM_DedicatedServer)W.SpawnActor<AShiftDirector>();}

AShiftAudioZone::AShiftAudioZone()
{
 Bounds=CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));RootComponent=Bounds;Bounds->SetBoxExtent(FVector(400,250,180));Bounds->SetCollisionProfileName(TEXT("Trigger"));
 Ambient=CreateDefaultSubobject<UAudioComponent>(TEXT("Ambient"));Ambient->SetupAttachment(RootComponent);Ambient->bAutoActivate=false;
 Story=CreateDefaultSubobject<UAudioComponent>(TEXT("Story"));Story->SetupAttachment(RootComponent);Story->bAutoActivate=false;
}
void AShiftAudioZone::BeginPlay(){Super::BeginPlay();Bounds->OnComponentBeginOverlap.AddDynamic(this,&AShiftAudioZone::Enter);Bounds->OnComponentEndOverlap.AddDynamic(this,&AShiftAudioZone::Leave);Ambient->SetSound(AmbientSound);}
void AShiftAudioZone::Enter(UPrimitiveComponent*,AActor* A,UPrimitiveComponent*,int32,bool,const FHitResult&){if(A==UGameplayStatics::GetPlayerPawn(this,0)){if(AmbientSound)Ambient->FadeIn(FadeSeconds,AmbientVolume);if(bTriggerStoryOnEnter)PlayStory(false);}}
void AShiftAudioZone::Leave(UPrimitiveComponent*,AActor* A,UPrimitiveComponent*,int32){if(A==UGameplayStatics::GetPlayerPawn(this,0)){Ambient->FadeOut(FadeSeconds,0);Story->FadeOut(FadeSeconds,0);}}
void AShiftAudioZone::PlayStory(bool Tension){USoundBase* S=Tension?TensionSound.Get():StorySound.Get();if(bPlayed||!S)return;bPlayed=true;Story->SetSound(S);Story->FadeIn(FadeSeconds,.35f);}
