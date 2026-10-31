#include "LabGame.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void ALabCat::ConfigureKeyboardCamera(){
 Arm->bUsePawnControlRotation=false;Arm->SetUsingAbsoluteRotation(true);
 Arm->SetWorldRotation(FRotator(-38,-35,0));Arm->TargetArmLength=(FixedCamera?760:620)*FMath::Sqrt(BodyScale);
 Arm->TargetOffset=FVector(0,0,45);Arm->bEnableCameraRotationLag=false;
}
FVector ALabCat::ScreenForward() const{return FRotator(0,Arm->GetTargetRotation().Yaw,0).Vector();}
void ALabCat::MenuUp(){if(auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode()))G->NavigateMenu(0,-1);}
void ALabCat::MenuDown(){if(auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode()))G->NavigateMenu(0,1);}
void ALabCat::MenuLeft(){if(auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode()))G->NavigateMenu(-1,0);}
void ALabCat::MenuRight(){if(auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode()))G->NavigateMenu(1,0);}

FName ALabGameMode::FocusedMenuAction() const{
 if(UGameplayStatics::IsGamePaused(GetWorld())){static const FName Actions[]={TEXT("Resume"),TEXT("Retry"),TEXT("Select"),TEXT("Shake")};return Actions[FMath::Clamp(MenuCursor,0,3)];}
 if(Stage==6)return MenuCursor%2?TEXT("TitleFree"):TEXT("TitleStart");
 if(Stage==4||Stage==5)return TEXT("Confirm");
 if(Stage==3){if(MemeShowing||MemeGifShowing)return TEXT("Meme");static const FName Actions[]={TEXT("Select"),TEXT("Retry"),TEXT("Meme")};return Actions[FMath::Clamp(MenuCursor,0,2)];}
 return NAME_None;
}
void ALabGameMode::NavigateMenu(int DX,int DY){
 const bool Paused=UGameplayStatics::IsGamePaused(GetWorld());
 if(!Paused&&(Stage==4||Stage==5)){
  const int Start=Stage==4?SelectedCat:PartnerCat,Step=DX?DX:DY*2;
  auto Allowed=[&](int I){return Unlocked(I)&&(Stage!=5||I!=SelectedCat);};
  for(int I=1;I<=4;I++){const int Candidate=(Start+Step*I+16)%4;if(Candidate!=Start&&Allowed(Candidate)){SelectCat(Candidate);return;}}
  // If a row or column is locked, reach the next available cat rather than dead-end.
  const int Direction=(DX+DY)>0?1:-1;
  for(int I=1;I<=4;I++){const int Candidate=(Start+Direction*I+8)%4;if(Allowed(Candidate)){SelectCat(Candidate);return;}}
  return;
 }
 const int Count=Paused?4:Stage==6?2:Stage==3&&!MemeShowing&&!MemeGifShowing?3:0;
 if(Count)MenuCursor=(MenuCursor+(DX?DX:DY)+Count)%Count;
}
void ALabGameMode::ActivateMenu(){
 if(!UGameplayStatics::IsGamePaused(GetWorld())&&(Stage==4||Stage==5)){ConfirmSelection();return;}
 auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));const FName Action=FocusedMenuAction();
 if(Action==TEXT("TitleStart"))StartFromTitle();
 else if(Action==TEXT("TitleFree"))StartFromTitle(true);
 else if(Action==TEXT("Resume")){if(Cat)Cat->Pause();}
 else if(Action==TEXT("Retry")){UGameplayStatics::SetGamePaused(GetWorld(),false);Retry();}
 else if(Action==TEXT("Select")){UGameplayStatics::SetGamePaused(GetWorld(),false);OpenSelection();}
 else if(Action==TEXT("Shake")){if(Cat)Cat->ToggleShake();}
 else if(Action==TEXT("Meme"))ToggleMemeGif();
}
