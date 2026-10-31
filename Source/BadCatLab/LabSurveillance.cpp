#include "LabGame.h"
#include "ProceduralMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

void ALabGameMode::CreateCameraFootprint(){
 auto A=GetWorld()->SpawnActor<AStaticMeshActor>();PresentationCats.Add(A);
 CameraVision=NewObject<UProceduralMeshComponent>(A);CameraVision->SetupAttachment(A->GetRootComponent());CameraVision->RegisterComponent();
 CameraVision->SetCollisionEnabled(ECollisionEnabled::NoCollision);CameraVision->SetCastShadow(false);CameraVision->bReceivesDecals=false;
 CameraVision->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/M_CameraFootprint")));
}

void ALabGameMode::UpdateCameraFootprint(){
 if(!IsValid(CameraVision)||!IsValid(PetCamera))return;
 CameraVision->SetVisibility(Playing()&&!CameraBroken);if(!Playing()||CameraBroken)return;
 // Depth-tested floor graphics, never a volume in front of the player camera.
 // Each ray stops at static cover; loose props retain their own sight-line check.
 const FVector O=PetCamera->GetActorLocation()+WatchDirection*20,Base(O.X,O.Y,2.5);
 const float Yaw=WatchDirection.Rotation().Yaw;
 constexpr int Segments=48;
 TArray<FVector> Edge,V,N;TArray<int32> T;TArray<FVector2D> UV;TArray<FLinearColor> Colors;
 FCollisionObjectQueryParams Objects(ECC_WorldStatic);FCollisionQueryParams Query;Query.AddIgnoredActor(PetCamera);
 for(int I=0;I<=Segments;I++){
  const FVector D=FRotator(0,Yaw-CameraHalfAngle+2*CameraHalfAngle*I/Segments,0).Vector();
  FHitResult Hit;const FVector End=FVector(O.X,O.Y,35)+D*CameraRange;
  const bool Block=GetWorld()->LineTraceSingleByObjectType(Hit,O,End,Objects,Query);
  const float Distance=Block?FMath::Clamp(FVector::Dist2D(O,Hit.Location)-5,0.f,CameraRange):CameraRange;
  Edge.Add(Base+D*Distance);
 }
 const bool Warning=Exposure>0||CameraFlash>0;
 const FLinearColor Tint=CameraFlash>0?FLinearColor(1,.12,.035):Warning?FLinearColor(1,.38,.08):FLinearColor(.65,.9,1);
 auto Triangle=[&](FVector A,FVector B,FVector C,float Alpha){int Start=V.Num();V.Append({A,B,C});T.Append({Start,Start+1,Start+2});for(int I=0;I<3;I++){N.Add(FVector::UpVector);UV.Add(FVector2D::ZeroVector);Colors.Add(FLinearColor(Tint.R,Tint.G,Tint.B,Alpha));}};
 auto Line=[&](FVector A,FVector B,float Width,float Alpha){if(FVector::DistSquared2D(A,B)<1)return;const FVector Side=FVector::CrossProduct(B-A,FVector::UpVector).GetSafeNormal()*Width*.5f;A.Z=B.Z=2.7;Triangle(A-Side,A+Side,B+Side,Alpha);Triangle(A-Side,B+Side,B-Side,Alpha);};
 for(int I=0;I<Segments;I++){
  Triangle(Base,Edge[I],Edge[I+1],Warning?.15f:.065f);
  Line(Edge[I],Edge[I+1],1.8f,.8f);
  // A broken inner arc suggests a surveillance scan without striping the room.
  if(I%6<3)Line(FMath::Lerp(Base,Edge[I],.72f),FMath::Lerp(Base,Edge[I+1],.72f),.9f,.35f);
 }
 for(int I:{0,Segments})Line(Base,Edge[I],1.2f,.55f);
 CameraVision->CreateMeshSection_LinearColor(0,V,T,N,UV,Colors,TArray<FProcMeshTangent>(),false);
}
