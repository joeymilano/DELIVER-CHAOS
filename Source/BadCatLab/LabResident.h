#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LabResident.generated.h"
UCLASS()
class BADCATLAB_API ALabResident : public ACharacter {
 GENERATED_BODY()
public:
 ALabResident();virtual void Tick(float Dt) override;
 void Configure(bool Guest);void ResetResident();bool AdvanceCapture(float Dt,bool Eligible);
 void BuildGroundGrid();bool CanStandAt(FVector Position) const;bool GroundSegmentClear(FVector From,FVector To) const;
 bool PlanGroundRoute(FVector Target);void NavigateFloor(FVector Target,float Dt);
 TArray<FBox> FurnitureFootprints;TArray<uint8> GroundGrid;TArray<FVector> GroundPath;FVector RequestedGoal,FloorGoal,LastSafeFloor,ProgressOrigin;
 float RouteCooldown=0,ProgressClock=0,BlockedPause=0;int PathPoint=0;
 static constexpr int GridWidth=43,GridHeight=33;static constexpr float GridStep=40,FloorCentre=92;
 FVector GroundCell(int Index) const;int NearestGroundCell(FVector Position) const;
 bool Guest=false;float Alert=0,Confused=0,CatchCooldown=0,StepClock=0,CaptureTime=0;int Point=0,Anim=-1;FVector Home;
 UPROPERTY() TArray<UAnimSequence*> Clips;
};
