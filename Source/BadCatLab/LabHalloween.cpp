#include "LabGame.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void ALabGameMode::BuildHalloweenDecor(){
 // Room dressing is built once, independently of round props and resident navigation.
 for(auto A:HalloweenDecor)if(IsValid(A))A->Destroy();HalloweenDecor.Empty();
 auto Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/Halloween/M_Sticker"));if(!Material)return;
 auto Sticker=[&](int Tile,FVector Centre,FVector Right,FVector Up,float Width,float Height,float Tilt=0){
  Right=Right.GetSafeNormal();Up=Up.GetSafeNormal();const FVector Normal=FVector::CrossProduct(Right,Up).GetSafeNormal();
  Right=Right.RotateAngleAxis(Tilt,Normal);Up=Up.RotateAngleAxis(Tilt,Normal);
  auto A=GetWorld()->SpawnActor<AActor>(Centre,FRotator::ZeroRotator);HalloweenDecor.Add(A);A->Tags.Add(TEXT("HalloweenSticker"));
  auto C=NewObject<UProceduralMeshComponent>(A);A->SetRootComponent(C);C->RegisterComponent();A->SetActorLocation(Centre);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCastShadow(false);C->bReceivesDecals=false;C->SetCanEverAffectNavigation(false);
  const FVector R=Right*Width*.5f,U=Up*Height*.5f;
  const TArray<FVector> V={-R+U,R+U,R-U,-R-U};
  // Unreal front faces are clockwise. Correct winding keeps two-sided lighting
  // from flipping these outward normals into the wall or underneath the carpet.
  C->CreateMeshSection_LinearColor(0,V,TArray<int32>{0,1,2,0,2,3},TArray<FVector>{Normal,Normal,Normal,Normal},TArray<FVector2D>{{0,0},{1,0},{1,1},{0,1}},TArray<FLinearColor>{FLinearColor::White,FLinearColor::White,FLinearColor::White,FLinearColor::White},TArray<FProcMeshTangent>(),false);
  auto M=UMaterialInstanceDynamic::Create(Material,A);M->SetScalarParameterValue(TEXT("TileX"),(Tile%2)*.5f);M->SetScalarParameterValue(TEXT("TileY"),(Tile/2)*.5f);C->SetMaterial(0,M);
 };
 const FVector Z=FVector::UpVector;
 // Sofa wall: a large ghost, a companion spider, and a corner web above the steps.
 Sticker(0,FVector(-430,-688.5f,230),-FVector::ForwardVector,Z,152,165,-6);
 Sticker(1,FVector(-215,-688.5f,290),-FVector::ForwardVector,Z,78,78,8);
 Sticker(2,FVector(-785,-688.5f,325),-FVector::ForwardVector,Z,132,132);
 Sticker(0,FVector(300,-688.5f,344),-FVector::ForwardVector,Z,88,96,8);
 // Step fronts are in the default camera's view, unlike the high perimeter walls.
 Sticker(2,FVector(-605,-471.5f,37),-FVector::ForwardVector,Z,66,58);
 Sticker(0,FVector(-415,-471.5f,78),-FVector::ForwardVector,Z,76,93,-4);
 Sticker(1,FVector(-225,-471.5f,152),-FVector::ForwardVector,Z,72,72,7);
 // Side walls remain legible from the ground and upper catwalk.
 Sticker(0,FVector(-888.5f,-230,220),FVector::RightVector,Z,130,142,9);
 Sticker(1,FVector(-888.5f,-420,285),FVector::RightVector,Z,68,68,-8);
 Sticker(2,FVector(888.5f,-590,320),-FVector::RightVector,Z,142,142);
 Sticker(0,FVector(888.5f,390,275),-FVector::RightVector,Z,110,120,-8);
 Sticker(1,FVector(888.5f,190,333),-FVector::RightVector,Z,72,72,7);
 // Tea-party wall: two accents, leaving the garlands and central fireplace open.
 Sticker(0,FVector(470,688.5f,260),FVector::ForwardVector,Z,96,105,5);
 Sticker(1,FVector(-480,688.5f,265),FVector::ForwardVector,Z,66,66,-7);
 // Printed on the teal carpet, below feet and the separate surveillance graphic.
 Sticker(3,FVector(-153,-93,1.85f),FVector::ForwardVector,FVector::RightVector,218,174,14);
}
