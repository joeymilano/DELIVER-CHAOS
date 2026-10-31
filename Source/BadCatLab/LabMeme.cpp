#include "LabGame.h"
#include "LabCatalog.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Engine/SkeletalMesh.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

// A quiet meow, with no whole-body spin or camera motion.
FRotator ALabGameMode::MemePose(float Time) { return FRotator::ZeroRotator; }
void ALabGameMode::ToggleMemeGif(){
 if(Stage!=3)return;
 if(MemeShowing){StopMeme();return;}
 if(!MemeGifTexture)MemeGifTexture=LoadObject<UTexture2D>(nullptr,TEXT("/Game/UI/MemesLocal/OiiaAtlas"),nullptr,LOAD_Quiet|LOAD_NoWarn);
 MemeGifShowing=MemeGifTexture&&!MemeGifShowing;MemeGifElapsed=0;
}

void ALabGameMode::StopMeme() {
 const bool Receipt=MemeShowing&&Stage==3;
 const bool Greet=MemeShowing&&ReceiptGreetingPending&&Stage==3;ReceiptGreetingPending=false;
 MemeShowing=false;
 if(IsValid(MemeAudio)){MemeAudio->Stop();MemeAudio->DestroyComponent();}MemeAudio=nullptr;
 if(auto PC=GetWorld()->GetFirstPlayerController())if(auto Pawn=PC->GetPawn())PC->SetViewTarget(Pawn);
 if(Greet)Speak(TEXT("OwnerHome"));
 if(Receipt)WorldSound(Score>0?TEXT("Win"):TEXT("Cash"),.55f);
}
void ALabGameMode::ClearMeme() {
 ReceiptGreetingPending=false;StopMeme();
 for(auto A:MemeSet)if(IsValid(A))A->Destroy();MemeSet.Empty();MemeCat=nullptr;MemeCamera=nullptr;MemeElapsed=0;MeowBody=nullptr;MeowPlayed=false;MemeGifShowing=false;MemeGifElapsed=0;
}

void ALabGameMode::StartMeme() {
 if(Stage!=3)return;
 ClearVoices();ClearMeme();
 UStaticMesh* Mesh=LoadObject<UStaticMesh>(nullptr,LabCatalog::Cat(SelectedCat).Mesh);if(!Mesh)return;
 const FVector Origin(0,0,1200);
 MemeCat=GetWorld()->SpawnActor<AActor>();MemeSet.Add(MemeCat);
 auto Root=NewObject<USceneComponent>(MemeCat);MemeCat->SetRootComponent(Root);Root->RegisterComponent();MemeCat->SetActorLocation(Origin);
 auto Body=NewObject<UStaticMeshComponent>(MemeCat);Body->SetupAttachment(Root);Body->SetStaticMesh(Mesh);Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);Body->RegisterComponent();
 const FBox Bounds=Mesh->GetBoundingBox();const float Scale=90.f/FMath::Max(1.f,Bounds.GetSize().Z);
 Body->SetRelativeScale3D(FVector(Scale));Body->SetRelativeLocation(FVector(0,0,-Bounds.Min.Z*Scale));Body->SetRelativeRotation(FRotator(0,-90,0));
 if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));Cat&&Cat->GetMesh()->GetSkeletalMeshAsset()){
  MeowBody=NewObject<UPoseableMeshComponent>(MemeCat);MeowBody->SetupAttachment(Root);MeowBody->SetSkeletalMesh(Cat->GetMesh()->GetSkeletalMeshAsset());MeowBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);MeowBody->RegisterComponent();
  const auto B=Cat->GetMesh()->GetSkeletalMeshAsset()->GetImportedBounds().GetBox();const float K=90.f/FMath::Max(1.f,B.GetSize().Z);
  MeowBody->SetRelativeScale3D(FVector(K));MeowBody->SetRelativeLocation(FVector(0,0,-B.Min.Z*K));MeowBody->SetRelativeRotation(FRotator(0,-90,0));
  MeowHeadRest=MeowBody->GetBoneRotationByName(TEXT("Head"),EBoneSpaces::ComponentSpace);MeowJawRest=MeowBody->GetBoneRotationByName(TEXT("Jaw"),EBoneSpaces::ComponentSpace);Body->SetVisibility(false);
 }
 // Keep actual fusion accessories so this is the run's cat, not a generic meme.
 if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){
  for(auto Source:Cat->FusionParts)if(IsValid(Source)&&Source->GetStaticMesh()){
   auto Part=NewObject<UStaticMeshComponent>(MemeCat);Part->SetupAttachment(Root);Part->SetStaticMesh(Source->GetStaticMesh());Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->RegisterComponent();
   const float Factor=90.f/(64.f*Cat->BodyScale);Part->SetRelativeLocation((Source->GetRelativeLocation()+FVector(0,0,30*Cat->BodyScale))*Factor);
   Part->SetRelativeRotation(Source->GetRelativeRotation());Part->SetRelativeScale3D(Source->GetRelativeScale3D()*Factor);
   for(int I=0;I<Source->GetNumMaterials();I++)Part->SetMaterial(I,Source->GetMaterial(I));
  }
 }
 auto SetPiece=[&](const TCHAR* Shape,FVector Pos,FVector Size,FLinearColor Color){
  auto A=GetWorld()->SpawnActor<AStaticMeshActor>(Pos,FRotator::ZeroRotator);MemeSet.Add(A);auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);
  C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s"),Shape)));C->SetWorldScale3D(Size);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  if(auto M=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/M_Palette"))){auto D=UMaterialInstanceDynamic::Create(M,C);D->SetVectorParameterValue(TEXT("Color"),Color);C->SetMaterial(0,D);}return A;
 };
 SetPiece(TEXT("Cylinder"),Origin-FVector(0,0,5),FVector(1.5,1.5,.1),FLinearColor(1,.145,.025));
 SetPiece(TEXT("Cube"),Origin+FVector(-140,0,100),FVector(.2,16,12),FLinearColor(.13,.22,.24));
 auto Light=GetWorld()->SpawnActor<APointLight>(Origin+FVector(150,-100,180),FRotator::ZeroRotator);MemeSet.Add(Light);
 Light->PointLightComponent->SetIntensity(120000);Light->PointLightComponent->SetAttenuationRadius(750);Light->PointLightComponent->SetCastShadows(false);
 MemeCamera=GetWorld()->SpawnActor<ACameraActor>();MemeSet.Add(MemeCamera);
 const FVector CameraPos=Origin+FVector(340,0,66);MemeCamera->SetActorLocation(CameraPos);MemeCamera->SetActorRotation((Origin+FVector(0,0,42)-CameraPos).Rotation());MemeCamera->GetCameraComponent()->FieldOfView=50;
 if(auto PC=GetWorld()->GetFirstPlayerController())PC->SetViewTarget(MemeCamera);
 MemeShowing=true;MemeElapsed=0;

}
void ALabGameMode::TickMeme(float Dt) {
 if(MemeGifShowing&&Stage==3){MemeGifElapsed+=FMath::Max(0.f,Dt);if(MemeGifElapsed>=5.34f)MemeGifShowing=false;}
 if(!MemeShowing||Stage!=3)return;
 MemeElapsed+=FMath::Max(0.f,Dt);
 if(MemeElapsed>=MemeDuration){
#if !UE_BUILD_SHIPPING
  if(NoSave&&FParse::Param(FCommandLine::Get(),TEXT("MemeReview"))){MemeElapsed=FMath::Fmod(MemeElapsed,MemeDuration);}
  else
#endif
  {StopMeme();return;}
 }
 if(!MeowPlayed&&MemeElapsed>=.35f){MeowPlayed=true;if(auto Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/MeowSoft")))MemeAudio=UGameplayStatics::SpawnSound2D(GetWorld(),Sound,.65f);}
 if(IsValid(MeowBody)){
  const float Lift=FMath::Sin(FMath::Clamp((MemeElapsed-.1f)/1.8f,0.f,1.f)*PI);
  const float Mouth=FMath::Sin(FMath::Clamp((MemeElapsed-.35f)/.75f,0.f,1.f)*PI);
  MeowBody->SetBoneRotationByName(TEXT("Head"),MeowHeadRest+FRotator(0,0,-7*Lift),EBoneSpaces::ComponentSpace);
  MeowBody->SetBoneRotationByName(TEXT("Jaw"),MeowJawRest+FRotator(0,0,9*Mouth),EBoneSpaces::ComponentSpace);
 }
 if(IsValid(MemeCat))MemeCat->SetActorRotation(MemePose(MemeElapsed));
}
