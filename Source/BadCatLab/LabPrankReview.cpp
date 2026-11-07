#include "LabGame.h"
#if !UE_BUILD_SHIPPING
#include "LabResident.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"

void ALabGameMode::SetupPrankReview(){
 if(!NoSave||!FParse::Value(FCommandLine::Get(),TEXT("PrankReview="),PrankReviewMode))return;
 Practice=true;SelectedCat=0;RecipeId=INDEX_NONE;StartRound(false);
 auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(this,0));if(!Cat)return;
 FVector Pos,Direction=FVector::ForwardVector;
 if(PrankReviewMode==TEXT("owner")){
  auto R=Residents[0];R->Home=FVector(-440,-200,92);R->ResetResident();R->Point=1;R->SetActorRotation(FRotator(0,90,0));
  Pos=R->GetActorLocation()-R->GetActorForwardVector()*80;Pos.Z=35;Direction=R->GetActorForwardVector();
 }else{Pos=PetCamera->GetActorLocation()-FVector(45,0,0);Pos.Z=35;}
 Cat->SetActorLocation(Pos);Cat->LastSafePosition=Pos;Cat->AimDirection=Direction;Cat->SetActorRotation(Direction.Rotation());
 Cat->Arm->SetWorldRotation(FRotator(-22,-35,0));Cat->Arm->TargetArmLength=350;Cat->Arm->TargetOffset=FVector(0,0,50);
 PrankReviewTime=0;PrankReviewStep=1;
}
void ALabGameMode::TickPrankReview(float Dt){
 if(PrankReviewStep<=0||!NoSave)return;
 auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(this,0));if(!Cat)return;
 PrankReviewTime+=Dt;
 FString Folder=FPaths::ProjectDir()/TEXT("Verification/pranks");IFileManager::Get().MakeDirectory(*Folder,true);
 if(PrankReviewStep==1&&PrankReviewTime>2.f){
  // Let the room, animation and physics settle before arranging the contact.
  // The owner keeps patrolling; no frozen character is used as the target.
  if(PrankReviewMode==TEXT("owner")){
   auto R=Residents[0];const FVector Direction=R->GetActorForwardVector();FVector Pos=R->GetActorLocation()-Direction*80;Pos.Z=32.2;
   Cat->GetCharacterMovement()->StopMovementImmediately();Cat->SetActorLocation(Pos);Cat->LastSafePosition=Pos;Cat->AimDirection=Direction;Cat->SetActorRotation(Direction.Rotation());
  }
  // Capturing a PNG here stalls the first jump frame and invalidates timing.
  if(PrankReviewMode!=TEXT("owner"))FScreenshotRequest::RequestScreenshot(Folder/(PrankReviewMode+TEXT("-before.png")),false,false);
  PrankReviewStep=2;
  if(PrankReviewMode==TEXT("owner")){Cat->Press();Cat->Release();PrankReviewStep=3;}
 }
 if(PrankReviewStep==2&&PrankReviewTime>2.2f){
  if(PrankReviewMode==TEXT("unplug"))Cat->PushStart();else{Cat->Press();Cat->Release();}
  PrankReviewStep=3;
 }
 if(PrankReviewStep==3&&(PrankPending||CameraBroken)){
  FScreenshotRequest::RequestScreenshot(Folder/(PrankReviewMode+TEXT("-contact.png")),false,false);PrankReviewStep=4;
 }
 if(PrankReviewTime>3.7f&&PrankReviewStep<5){
  FScreenshotRequest::RequestScreenshot(Folder/(PrankReviewMode+TEXT("-after.png")),false,false);
  const FString Report=FString::Printf(TEXT("{\"mode\":\"%s\",\"real_contact\":%s,\"prank_pending\":%s,\"camera_broken\":%s,\"score\":%d,\"cat\":\"%s\",\"shoulder\":\"%s\",\"forearm\":\"%s\",\"foot\":\"%s\"}"),*PrankReviewMode,PrankReviewStep==4?TEXT("true"):TEXT("false"),PrankPending?TEXT("true"):TEXT("false"),CameraBroken?TEXT("true"):TEXT("false"),Score,*Cat->GetActorLocation().ToString(),*Cat->GetMesh()->GetSocketLocation(TEXT("RightArm")).ToString(),*Cat->GetMesh()->GetSocketLocation(TEXT("RightForeArm")).ToString(),*Cat->GetMesh()->GetSocketLocation(TEXT("RightFrontFoot")).ToString());
  FFileHelper::SaveStringToFile(Report,*(Folder/(PrankReviewMode+TEXT(".json"))));PrankReviewStep=5;
 }
 if(PrankReviewTime>4.5f)FPlatformMisc::RequestExit(false);
}
#endif
