#include "LabResident.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"

FVector ALabResident::GroundCell(int Index) const {
 return FVector(-840+(Index%GridWidth)*GridStep,-640+(Index/GridWidth)*GridStep,FloorCentre);
}
bool ALabResident::CanStandAt(FVector Position) const {
 Position.Z=FloorCentre;
 for(const FBox& Box:FurnitureFootprints)if(Box.IsInsideOrOn(Position))return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(ResidentFloor),false,this);
 const FCollisionObjectQueryParams StaticObjects(ECC_WorldStatic);
 // A whole standing capsule excludes tabletops, stairs and low gaps, not just legs.
 if(GetWorld()->OverlapAnyTestByObjectType(Position,FQuat::Identity,StaticObjects,FCollisionShape::MakeCapsule(32,89),Q))return false;
 FHitResult Floor;
 return GetWorld()->LineTraceSingleByObjectType(Floor,FVector(Position.X,Position.Y,12),FVector(Position.X,Position.Y,-12),StaticObjects,Q)&&Floor.ImpactPoint.Z<=3&&Floor.ImpactNormal.Z>.9f;
}
bool ALabResident::GroundSegmentClear(FVector From,FVector To) const {
 From.Z=To.Z=FloorCentre;
 for(const FBox& Box:FurnitureFootprints)if(FMath::LineBoxIntersection(Box,From,To,To-From))return false;
 FCollisionQueryParams Q(SCENE_QUERY_STAT(ResidentRoute),false,this);FHitResult Hit;
 return !GetWorld()->SweepSingleByObjectType(Hit,From,To,FQuat::Identity,FCollisionObjectQueryParams(ECC_WorldStatic),FCollisionShape::MakeCapsule(32,89),Q);
}
void ALabResident::BuildGroundGrid(){
 // Imported meshes may have sparse leg-only collision. Humans still respect the
 // full furniture footprint, while cats retain their existing climbing collision.
 FurnitureFootprints.Empty();
 for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It){
  auto Mesh=It->GetStaticMeshComponent();if(!Mesh||Mesh->GetCollisionEnabled()==ECollisionEnabled::NoCollision)continue;
  FBox Bounds=Mesh->Bounds.GetBox();if(Bounds.Max.Z<=5||Bounds.Min.Z>=180)continue;
  Bounds=Bounds.ExpandBy(FVector(34,34,0));Bounds.Min.Z=0;Bounds.Max.Z=185;FurnitureFootprints.Add(Bounds);
 }
 GroundGrid.SetNumZeroed(GridWidth*GridHeight);
 for(int I=0;I<GroundGrid.Num();I++)GroundGrid[I]=CanStandAt(GroundCell(I));
 GroundPath.Empty();RouteCooldown=0;
}
int ALabResident::NearestGroundCell(FVector Position) const {
 int Best=INDEX_NONE;float Distance=MAX_flt;
 for(int I=0;I<GroundGrid.Num();I++)if(GroundGrid[I]){float D=FVector::DistSquared2D(Position,GroundCell(I));if(D<Distance){Distance=D;Best=I;}}
 return Best;
}
bool ALabResident::PlanGroundRoute(FVector Target){
 GroundPath.Empty();PathPoint=0;RequestedGoal=Target;
 if(GroundGrid.IsEmpty())BuildGroundGrid();
 int Start=NearestGroundCell(GetActorLocation());const int Goal=NearestGroundCell(Target);
 if(Start!=INDEX_NONE&&!GroundSegmentClear(GetActorLocation(),GroundCell(Start))){
  Start=INDEX_NONE;float Best=FMath::Square(160.f);
  for(int I=0;I<GroundGrid.Num();I++)if(GroundGrid[I]){float D=FVector::DistSquared2D(GetActorLocation(),GroundCell(I));if(D<Best&&GroundSegmentClear(GetActorLocation(),GroundCell(I))){Start=I;Best=D;}}
 }
 if(Start==INDEX_NONE||Goal==INDEX_NONE)return false;
 FloorGoal=GroundCell(Goal);
 if(!GroundSegmentClear(GetActorLocation(),GroundCell(Start)))return false;
 TArray<float> Cost;Cost.Init(MAX_flt,GroundGrid.Num());TArray<int> Parent;Parent.Init(INDEX_NONE,GroundGrid.Num());
 TArray<uint8> Closed;Closed.Init(0,GroundGrid.Num());TArray<int> Open;Open.Add(Start);Cost[Start]=0;
 auto Distance=[&](int A,int B){return FVector::Dist2D(GroundCell(A),GroundCell(B));};
 int End=Start;
 while(Open.Num()){
  int Best=0;for(int I=1;I<Open.Num();I++)if(Cost[Open[I]]+Distance(Open[I],Goal)<Cost[Open[Best]]+Distance(Open[Best],Goal))Best=I;
  const int Current=Open[Best];Open.RemoveAtSwap(Best);Closed[Current]=1;
  if(Distance(Current,Goal)<Distance(End,Goal))End=Current;
  if(Current==Goal){End=Goal;break;}
  const int X=Current%GridWidth,Y=Current/GridWidth;
  for(int DY=-1;DY<=1;DY++)for(int DX=-1;DX<=1;DX++){
   if((!DX&&!DY)||X+DX<0||X+DX>=GridWidth||Y+DY<0||Y+DY>=GridHeight)continue;
   const int Next=(Y+DY)*GridWidth+X+DX;if(!GroundGrid[Next]||Closed[Next])continue;
   if(DX&&DY&&(!GroundGrid[Y*GridWidth+X+DX]||!GroundGrid[(Y+DY)*GridWidth+X]))continue;
   const float NewCost=Cost[Current]+Distance(Current,Next);
   if(NewCost>=Cost[Next]||!GroundSegmentClear(GroundCell(Current),GroundCell(Next)))continue;
   Parent[Next]=Current;Cost[Next]=NewCost;Open.AddUnique(Next);
  }
 }
 // A cat on furniture is approached from the nearest reachable floor position.
 FloorGoal=GroundCell(End);
 TArray<FVector> Reverse;for(int At=End;At!=INDEX_NONE;At=Parent[At])Reverse.Add(GroundCell(At));
 for(int I=Reverse.Num()-1;I>=0;I--)GroundPath.Add(Reverse[I]);
 return GroundPath.Num()>0;
}
void ALabResident::NavigateFloor(FVector Target,float Dt){
 RouteCooldown=FMath::Max(0.f,RouteCooldown-Dt);BlockedPause=FMath::Max(0.f,BlockedPause-Dt);
 if(BlockedPause>0){GetCharacterMovement()->StopMovementImmediately();ConsumeMovementInputVector();return;}
 if(RouteCooldown<=0&&(GroundPath.IsEmpty()||FVector::DistSquared2D(Target,RequestedGoal)>6400)){
  PlanGroundRoute(Target);RouteCooldown=.75f;
 }
 while(GroundPath.IsValidIndex(PathPoint)&&FVector::Dist2D(GetActorLocation(),GroundPath[PathPoint])<6)PathPoint++;
 if(!GroundPath.IsValidIndex(PathPoint)){GroundPath.Empty();GetCharacterMovement()->StopMovementImmediately();ConsumeMovementInputVector();return;}
 // Smooth the route only when the complete capsule fits; never cut through a chair.
 while(GroundPath.IsValidIndex(PathPoint+1)&&GroundSegmentClear(GetActorLocation(),GroundPath[PathPoint+1]))PathPoint++;
 if(!GroundSegmentClear(GetActorLocation(),GroundPath[PathPoint])){
  GroundPath.Empty();BlockedPause=.6f;RouteCooldown=0;GetCharacterMovement()->StopMovementImmediately();ConsumeMovementInputVector();return;
 }
 const FVector Heading=(GroundPath[PathPoint]-GetActorLocation()).GetSafeNormal2D();
 // Brake before tight turns instead of coasting into the furniture clearance band.
 if(FVector::DotProduct(GetVelocity().GetSafeNormal2D(),Heading)<.65f)GetCharacterMovement()->StopMovementImmediately();
 AddMovementInput(Heading,FMath::Clamp(FVector::Dist2D(GetActorLocation(),GroundPath[PathPoint])/90.f,.22f,1.f));
 ProgressClock+=Dt;
 if(ProgressClock>=.8f){
  if(FVector::Dist2D(GetActorLocation(),ProgressOrigin)<8){GroundPath.Empty();BlockedPause=.6f;RouteCooldown=0;GetCharacterMovement()->StopMovementImmediately();ConsumeMovementInputVector();}
  ProgressOrigin=GetActorLocation();ProgressClock=0;
 }
}
