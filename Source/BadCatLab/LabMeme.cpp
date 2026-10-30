#include "LabGame.h"
#include "LabCatalog.h"
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

// Rigid, deliberately deadpan spins. The camera and collision never spin.
// Three complete-turn bursts separated by motionless, front-facing beats.
FRotator ALabGameMode::MemePose(float Time) {
 const float T=FMath::Clamp(Time,0.f,MemeDuration);
 auto Burst=[](float Value,float Start,float Length,int Turns){
  const float P=FMath::Clamp((Value-Start)/Length,0.f,1.f);
  return 360.f*Turns*P*P*(3-2*P);
 };
 const float Yaw=Burst(T,.50f,.80f,3)+Burst(T,1.65f,.95f,4)+Burst(T,3.f,1.15f,5);
 return FRotator(0,FMath::Fmod(Yaw,360.f),0);
}

void ALabGameMode::StopMeme() {
 const bool Greet=MemeShowing&&ReceiptGreetingPending&&Stage==3;ReceiptGreetingPending=false;
 MemeShowing=false;
 if(IsValid(MemeAudio)){MemeAudio->Stop();MemeAudio->DestroyComponent();}MemeAudio=nullptr;
 if(auto PC=GetWorld()->GetFirstPlayerController())if(auto Pawn=PC->GetPawn())PC->SetViewTarget(Pawn);
 if(Greet)Speak(TEXT("OwnerHome"));
}
void ALabGameMode::ClearMeme() {
 ReceiptGreetingPending=false;StopMeme();
 for(auto A:MemeSet)if(IsValid(A))A->Destroy();MemeSet.Empty();MemeCat=nullptr;MemeCamera=nullptr;MemeElapsed=0;
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
 // Optional audio cut from the user's reference. Local demo only, with a portable fallback.
 if(auto Sound=LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/LocalReference/OiiaReference"),nullptr,LOAD_Quiet|LOAD_NoWarn))MemeAudio=UGameplayStatics::SpawnSound2D(GetWorld(),Sound,.65f);
}
void ALabGameMode::TickMeme(float Dt) {
 if(!MemeShowing||Stage!=3)return;
 MemeElapsed+=FMath::Max(0.f,Dt);
 if(MemeElapsed>=MemeDuration){
#if !UE_BUILD_SHIPPING
  if(NoSave&&FParse::Param(FCommandLine::Get(),TEXT("MemeReview"))){MemeElapsed=FMath::Fmod(MemeElapsed,MemeDuration);}
  else
#endif
  {StopMeme();return;}
 }
 if(IsValid(MemeCat))MemeCat->SetActorRotation(MemePose(MemeElapsed));
}
