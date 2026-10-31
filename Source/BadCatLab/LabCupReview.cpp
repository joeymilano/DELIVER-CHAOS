#include "LabGame.h"
#if !UE_BUILD_SHIPPING
#include "LabResident.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HighResScreenshot.h"
#include "UnrealClient.h"

void ALabGameMode::SetupCupReview(){
 if(!NoSave||!FParse::Param(FCommandLine::Get(),TEXT("CupReview")))return;
 Practice=true;SelectedCat=0;RecipeId=INDEX_NONE;StartRound(false);HideHUD=true;
 auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));if(!Cat)return;
 FBox Table(ForceInit);
 for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It){auto C=It->GetStaticMeshComponent();
  if(C->GetStaticMesh()&&C->GetStaticMesh()->GetName()==TEXT("SM_Coffe_Table_2")){Table=C->Bounds.GetBox();break;}
 }
 if(!Table.IsValid)return;
 for(auto P:Props){if(P->PropId==0){ReviewCup=P;}else {P->Body->SetSimulatePhysics(false);P->SetActorEnableCollision(false);P->SetActorHiddenInGame(true);}}
 auto Cup=ReviewCup.Get();if(!Cup)return;
 const float X=Table.Max.X-9.f,Y=Table.GetCenter().Y;
 Cup->Body->SetSimulatePhysics(false);
 const float Bottom=Cup->Body->GetStaticMesh()->GetBoundingBox().Min.Z*Cup->Body->GetComponentScale().Z;
 Cup->SetActorLocation(FVector(X,Y,Table.Max.Z-Bottom+1));Cup->SetActorRotation(FRotator::ZeroRotator);
 Cup->InitialPosition=Cup->GetActorLocation();Cup->Body->SetSimulatePhysics(true);
 Cat->SetActorLocation(FVector(X-47,Y-4,Table.Max.Z+31));Cat->SetActorRotation(FRotator::ZeroRotator);Cat->AimDirection=FVector::ForwardVector;
 Cat->LastSafePosition=Cat->GetActorLocation();Cat->GetCharacterMovement()->StopMovementImmediately();
 Cat->Arm->SetWorldRotation(FRotator(-20,-62,0));Cat->Arm->TargetArmLength=290;Cat->Arm->TargetOffset=FVector(15,0,10);
 for(auto R:Residents)if(IsValid(R)){R->SetActorTickEnabled(false);R->GetCharacterMovement()->StopMovementImmediately();}
 CupReviewStep=1;CupReviewTime=0;
 UE_LOG(LogTemp,Display,TEXT("CUP_REVIEW setup table %s cup %s cat %s"),*Table.ToString(),*Cup->GetActorLocation().ToString(),*Cat->GetActorLocation().ToString());
}
void ALabGameMode::TickCupReview(float Dt){
 if(CupReviewStep<=0||!NoSave)return;
 auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));auto Cup=ReviewCup.Get();if(!Cat||!Cup)return;
 CupReviewTime+=Dt;
 if(CupReviewStep==1&&CupReviewTime>2&&!FParse::Param(FCommandLine::Get(),TEXT("CupManual"))){
  Cup->InitialPosition=Cup->GetActorLocation();Cat->PushStart();CupReviewStep=2;
  UE_LOG(LogTemp,Display,TEXT("CUP_REVIEW begin reachable=%d shoulder=%s forearm=%s foot=%s cup=%s"),Cat->FindPushTarget()==Cup,*Cat->GetMesh()->GetSocketLocation(TEXT("RightArm")).ToString(),*Cat->GetMesh()->GetSocketLocation(TEXT("RightForeArm")).ToString(),*Cat->GetMesh()->GetSocketLocation(TEXT("RightFrontFoot")).ToString(),*Cup->GetActorLocation().ToString());
 }
 if(CupReviewStep==2&&Cat->PushContactApplied){CupReviewStep=3;UE_LOG(LogTemp,Display,TEXT("CUP_REVIEW contact t=%.3f cup=%s velocity=%s"),CupReviewTime,*Cup->GetActorLocation().ToString(),*Cup->Body->GetPhysicsLinearVelocity().ToString());}
 if(CupReviewStep==3&&Cup->GetActorLocation().Z<Cup->InitialPosition.Z-10){CupReviewStep=4;UE_LOG(LogTemp,Display,TEXT("CUP_REVIEW fall t=%.3f"),CupReviewTime);}
 if(CupReviewStep==4&&Cat->Innocent){CupReviewStep=5;UE_LOG(LogTemp,Display,TEXT("CUP_REVIEW innocent t=%.3f score=%d"),CupReviewTime,Score);}
 if(CupReviewTime>8&&Cat->Pushing&&!FParse::Param(FCommandLine::Get(),TEXT("CupManual")))Cat->PushStop();
 // Actual rendered frames, requested by the local review flag; no player-save writes.
 static int32 LastFrame=INDEX_NONE;
 if(FParse::Param(FCommandLine::Get(),TEXT("CupFrames"))&&CupReviewTime>=1.7f&&CupReviewTime<6){
  int32 Frame=FMath::FloorToInt((CupReviewTime-1.7f)*30);
  if(Frame!=LastFrame){LastFrame=Frame;FScreenshotRequest::RequestScreenshot(FPaths::ProjectDir()/FString::Printf(TEXT("Verification/cat-contact/frames/frame-%04d.png"),Frame),false,false);}
 }
 if(CupReviewTime>10&&CupReviewStep!=6&&!FParse::Param(FCommandLine::Get(),TEXT("CupManual"))){
  FString Report=FString::Printf(TEXT("{\"contact\":%s,\"fall\":%s,\"innocent\":%s,\"score\":%d,\"seconds\":%.3f}"),CupReviewStep>=3?TEXT("true"):TEXT("false"),CupReviewStep>=4?TEXT("true"):TEXT("false"),CupReviewStep>=5?TEXT("true"):TEXT("false"),Score,CupReviewTime);
  FFileHelper::SaveStringToFile(Report,*(FPaths::ProjectDir()/TEXT("Verification/cat-contact/scene.json")));CupReviewStep=6;
 }
}
#endif
