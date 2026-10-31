#include "LabGame.h"
#include "LabCatalog.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkeletalMesh.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

void ALabGameMode::ClearTitle(){
 if(TitleSet.IsEmpty())return;
 if(auto PC=GetWorld()->GetFirstPlayerController())if(auto Pawn=PC->GetPawn())PC->SetViewTarget(Pawn);
 for(auto A:TitleSet)if(IsValid(A))A->Destroy();TitleSet.Empty();TitleBody=nullptr;TitleCamera=nullptr;TitleTime=0;TitleCredits=false;
}
void ALabGameMode::StartFromTitle(bool FreePlay){Practice=FreePlay;OpenSelection();}
void ALabGameMode::OpenTitle(){
 OpenSelection();Stage=6;TitleTime=0;SetMenuInput(true);
 const FVector O(0,0,2400);
 auto Mesh=[&](const TCHAR* Path,FVector Pos,float Height,FRotator Rot=FRotator::ZeroRotator){
  auto M=LoadObject<UStaticMesh>(nullptr,Path);if(!M)return (AStaticMeshActor*)nullptr;
  auto A=GetWorld()->SpawnActor<AStaticMeshActor>(O+Pos,Rot);TitleSet.Add(A);auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(M);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  float K=Height/FMath::Max(1.f,M->GetBoundingBox().GetSize().Z);A->SetActorScale3D(FVector(K));A->AddActorWorldOffset(FVector(0,0,-M->GetBoundingBox().Min.Z*K));return A;
 };
 auto Block=[&](FVector Pos,FVector Scale,FLinearColor Color){auto A=Mesh(TEXT("/Engine/BasicShapes/Cube"),Pos,100);A->SetActorScale3D(Scale);if(auto Mat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/M_Palette"))){auto D=UMaterialInstanceDynamic::Create(Mat,A);D->SetVectorParameterValue(TEXT("Color"),Color);A->GetStaticMeshComponent()->SetMaterial(0,D);}return A;};
 Block(FVector(0,0,-15),FVector(12,16,.12),FLinearColor(.66,.73,.69));
 Block(FVector(-200,0,0),FVector(.1,16,8),FLinearColor(.33,.48,.47));
 // Real imported furniture and small props; isolated from gameplay collisions.
 Mesh(TEXT("/Game/FabLocal/Furniture/SM_Sofa_41"),FVector(-95,130,0),100,FRotator(0,-90,0));
 Mesh(TEXT("/Game/FabLocal/Furniture/SM_Coffe_Table_2"),FVector(45,160,0),48);
 Mesh(TEXT("/Game/FabLocal/Decor/SM_SD_PottedPlant_01"),FVector(-90,310,0),130);
 Mesh(TEXT("/Game/Art/Props/SM_Cup"),FVector(60,203,49),19,FRotator(0,0,14));
 Mesh(TEXT("/Game/Art/Party/SM_Pumpkin"),FVector(95,280,0),40);
 auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
 if(Cat&&Cat->GetMesh()->GetSkeletalMeshAsset()){
  auto A=GetWorld()->SpawnActor<AActor>();TitleSet.Add(A);
  TitleBody=NewObject<UPoseableMeshComponent>(A);A->SetRootComponent(TitleBody);TitleBody->SetSkeletalMesh(Cat->GetMesh()->GetSkeletalMeshAsset());TitleBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);TitleBody->RegisterComponent();
  auto B=Cat->GetMesh()->GetSkeletalMeshAsset()->GetImportedBounds().GetBox();float K=90/FMath::Max(1.f,B.GetSize().Z);
  A->SetActorScale3D(FVector(K));A->SetActorRotation(FRotator(0,-68,0));A->SetActorLocation(O+FVector(45,130,49-B.Min.Z*K));
  TitleHeadRest=TitleBody->GetBoneRotationByName(TEXT("Head"),EBoneSpaces::ComponentSpace);TitleTailRest=TitleBody->GetBoneRotationByName(TEXT("Tail2"),EBoneSpaces::ComponentSpace);
 }else Mesh(LabCatalog::Cat(SelectedCat).Mesh,FVector(45,130,49),90,FRotator(0,-68,0));
 for(int I=0;I<2;I++){auto L=GetWorld()->SpawnActor<APointLight>(O+FVector(I?50:200,I?320:-80,260),FRotator::ZeroRotator);TitleSet.Add(L);L->PointLightComponent->SetIntensity(I?90000:180000);L->PointLightComponent->SetAttenuationRadius(950);L->PointLightComponent->SetCastShadows(I==0);L->PointLightComponent->SetLightColor(I?FLinearColor(.72,.86,1):FLinearColor(1,.86,.68));}
 TitleCamera=GetWorld()->SpawnActor<ACameraActor>();TitleSet.Add(TitleCamera);
 FVector P=O+FVector(390,100,210);TitleCamera->SetActorLocation(P);TitleCamera->SetActorRotation((O+FVector(20,235,85)-P).Rotation());TitleCamera->GetCameraComponent()->FieldOfView=45;
 if(auto PC=GetWorld()->GetFirstPlayerController())PC->SetViewTarget(TitleCamera);
}
void ALabGameMode::TickTitle(float Dt){
 if(Stage!=6||!IsValid(TitleBody))return;TitleTime+=Dt;
 TitleBody->SetBoneRotationByName(TEXT("Head"),TitleHeadRest+FRotator(0,0,3*FMath::Sin(TitleTime*.7f)),EBoneSpaces::ComponentSpace);
 TitleBody->SetBoneRotationByName(TEXT("Tail2"),TitleTailRest+FRotator(0,8*FMath::Sin(TitleTime*1.2f),0),EBoneSpaces::ComponentSpace);
}
