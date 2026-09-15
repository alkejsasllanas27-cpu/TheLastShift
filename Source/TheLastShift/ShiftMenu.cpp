#include "ShiftGameplay.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"
#include "InputCoreTypes.h"

void UShiftMenuWidget::NativeOnInitialized()
{
 Super::NativeOnInitialized();SetIsFocusable(true);
 auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
 auto* Shade=WidgetTree->ConstructWidget<UBorder>();Shade->SetBrushColor(FLinearColor(.012f,.018f,.019f,.97f));auto* Fill=Canvas->AddChildToCanvas(Shade);Fill->SetAnchors(FAnchors(0,0,1,1));Fill->SetOffsets(FMargin(0));
 auto* List=WidgetTree->ConstructWidget<UVerticalBox>();auto* MenuSlot=Canvas->AddChildToCanvas(List);MenuSlot->SetAnchors(FAnchors(.5f,.5f));MenuSlot->SetAlignment(FVector2D(.5f,.5f));MenuSlot->SetSize(FVector2D(500,630));
 auto Label=[&](const FString& Value,int Font){auto* T=WidgetTree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Value));T->SetFont(FCoreStyle::GetDefaultFontStyle("Regular",Font));T->SetColorAndOpacity(FLinearColor(.83f,.85f,.79f));T->SetJustification(ETextJustify::Center);T->SetAutoWrapText(true);return T;};
 List->AddChildToVerticalBox(Label(TEXT("THE LAST SHIFT"),38))->SetPadding(FMargin(0,0,0,8));
 List->AddChildToVerticalBox(Label(TEXT("NIGHT DUTY  /  ABANDONED HOSPITAL"),12))->SetPadding(FMargin(0,0,0,28));
 auto Button=[&](const FString& Name,UTextBlock*& Text){auto* B=WidgetTree->ConstructWidget<UButton>();Text=Label(Name,17);Cast<UButtonSlot>(B->AddChild(Text))->SetPadding(FMargin(15,11));B->SetBackgroundColor(FLinearColor(.09f,.12f,.12f));auto* S=List->AddChildToVerticalBox(B);S->SetPadding(FMargin(0,4));return B;};
 UTextBlock* T=nullptr;Button(TEXT("PLAY / RESUME"),T)->OnClicked.AddDynamic(this,&UShiftMenuWidget::Resume);
 Button(TEXT(""),T)->OnClicked.AddDynamic(this,&UShiftMenuWidget::Quality);QualityText=T;
 Button(TEXT(""),T)->OnClicked.AddDynamic(this,&UShiftMenuWidget::VSync);VSyncText=T;
 Button(TEXT(""),T)->OnClicked.AddDynamic(this,&UShiftMenuWidget::WindowMode);ModeText=T;
 Button(TEXT(""),T)->OnClicked.AddDynamic(this,&UShiftMenuWidget::Resolution);ResolutionText=T;
 Button(TEXT(""),T)->OnClicked.AddDynamic(this,&UShiftMenuWidget::FrameLimit);FrameText=T;
 Button(TEXT("QUIT"),T)->OnClicked.AddDynamic(this,&UShiftMenuWidget::Quit);
 List->AddChildToVerticalBox(Label(TEXT("WASD MOVE   ·   SHIFT RUN   ·   E INTERACT\nF FLASHLIGHT   ·   R BATTERY   ·   P MENU"),12))->SetPadding(FMargin(0,22,0,0));
 List->AddChildToVerticalBox(Label(TEXT("Drawer: Poly Haven / James Ray Cock · CC0\nDrawer recordings: Joseph Sardin / BigSoundBank · CC0"),10))->SetPadding(FMargin(0,15,0,0));
 Refresh();
}
void UShiftMenuWidget::Refresh()
{
 auto* S=UGameUserSettings::GetGameUserSettings();if(!S)return;
 const TCHAR* Names[]={TEXT("LOW"),TEXT("MEDIUM"),TEXT("HIGH"),TEXT("EPIC"),TEXT("CINEMATIC")};int32 Q=S->GetOverallScalabilityLevel();
 QualityText->SetText(FText::FromString(FString::Printf(TEXT("GRAPHICS    %s    >"),Q>=0&&Q<5?Names[Q]:TEXT("CUSTOM"))));
 VSyncText->SetText(FText::FromString(S->IsVSyncEnabled()?TEXT("V-SYNC    ON    >"):TEXT("V-SYNC    OFF    >")));
 const TCHAR* Modes[]={TEXT("FULLSCREEN"),TEXT("BORDERLESS"),TEXT("WINDOWED")};ModeText->SetText(FText::FromString(FString::Printf(TEXT("DISPLAY    %s    >"),Modes[FMath::Clamp(int32(S->GetFullscreenMode()),0,2)])));
 auto R=S->GetScreenResolution();ResolutionText->SetText(FText::FromString(FString::Printf(TEXT("RESOLUTION    %d × %d    >"),R.X,R.Y)));
 FrameText->SetText(FText::FromString(S->GetFrameRateLimit()<=0?FString(TEXT("FRAME LIMIT    UNLIMITED    >")):FString::Printf(TEXT("FRAME LIMIT    %d FPS    >"),FMath::RoundToInt(S->GetFrameRateLimit()))));
}
void UShiftMenuWidget::Resume(){if(Director&&Director->bMenuOpen)Director->ToggleMenu();}
void UShiftMenuWidget::Quality(){auto* S=UGameUserSettings::GetGameUserSettings();S->SetOverallScalabilityLevel((FMath::Max(0,S->GetOverallScalabilityLevel())+1)%4);S->ApplyNonResolutionSettings();S->SaveSettings();Refresh();}
void UShiftMenuWidget::VSync(){auto* S=UGameUserSettings::GetGameUserSettings();S->SetVSyncEnabled(!S->IsVSyncEnabled());S->ApplyNonResolutionSettings();S->SaveSettings();Refresh();}
void UShiftMenuWidget::WindowMode(){auto* S=UGameUserSettings::GetGameUserSettings();S->SetFullscreenMode(S->GetFullscreenMode()==EWindowMode::Windowed?EWindowMode::WindowedFullscreen:EWindowMode::Windowed);S->ApplyResolutionSettings(false);S->ConfirmVideoMode();S->SaveSettings();Refresh();}
void UShiftMenuWidget::Resolution(){auto* S=UGameUserSettings::GetGameUserSettings();int32 X=S->GetScreenResolution().X;S->SetScreenResolution(X<1600?FIntPoint(1600,900):X<1920?FIntPoint(1920,1080):FIntPoint(1280,720));S->ApplyResolutionSettings(false);S->ConfirmVideoMode();S->SaveSettings();Refresh();}
void UShiftMenuWidget::FrameLimit(){auto* S=UGameUserSettings::GetGameUserSettings();float F=S->GetFrameRateLimit();S->SetFrameRateLimit(F<60?60:F<90?90:F<120?120:30);S->ApplyNonResolutionSettings();S->SaveSettings();Refresh();}
void UShiftMenuWidget::Quit(){UKismetSystemLibrary::QuitGame(this,GetOwningPlayer(),EQuitPreference::Quit,false);}
FReply UShiftMenuWidget::NativeOnKeyDown(const FGeometry& G,const FKeyEvent& E){if(E.GetKey()==EKeys::P||E.GetKey()==EKeys::Escape){Resume();return FReply::Handled();}return Super::NativeOnKeyDown(G,E);}
