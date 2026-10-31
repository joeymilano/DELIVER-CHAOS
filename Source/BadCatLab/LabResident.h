#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LabResident.generated.h"
UCLASS()
class BADCATLAB_API ALabResident : public ACharacter {
 GENERATED_BODY()
public:
 ALabResident();virtual void Tick(float Dt) override;
 void Configure(bool Guest);void ResetResident();
 bool Guest=false;float Alert=0,Confused=0,CatchCooldown=0,StepClock=0;int Point=0,Anim=-1;FVector Home;
 UPROPERTY() TArray<UAnimSequence*> Clips;
};
