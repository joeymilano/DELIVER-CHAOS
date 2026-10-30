#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "LabGame.generated.h"
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UPhysicalMaterial;
class UFont;
USTRUCT()
struct FLabChain {
 GENERATED_BODY()
 int32 Id=0; bool Pounce=false; double Last=0; TSet<int32> Members;
};
UCLASS()
class BADCATLAB_API ALabProp : public AActor {
 GENERATED_BODY()
public:
 ALabProp(); virtual void Tick(float Dt) override;
 UPROPERTY() UStaticMeshComponent* Body;
 UPROPERTY() UPhysicalMaterial* Physics;
 int32 PropId=0, ChainId=0, BaseScore=100, Bounces=0;
 bool Scored=false, Heavy=false, Mutated=false;
 void Configure(int32 Id,FVector Scale,FLinearColor Color,bool Ball,bool IsHeavy,bool Bounce);
 void Strike(int32 Chain,FVector Velocity);
 UFUNCTION() void Hit(UPrimitiveComponent* HitComp,AActor* Other,UPrimitiveComponent* OtherComp,FVector Impulse,const FHitResult& Result);
};
UCLASS()
class BADCATLAB_API ALabCat : public ACharacter {
 GENERATED_BODY()
public:
 ALabCat(); virtual void BeginPlay() override; virtual void Tick(float Dt) override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 UPROPERTY() USpringArmComponent* Arm;
 UPROPERTY() UCameraComponent* Camera;
 UPROPERTY() USceneComponent* Visual;
 UPROPERTY() TArray<UStaticMeshComponent*> Paws;
 UPROPERTY() UStaticMeshComponent* Tail;
 float Charge=0, ActionTime=0, Idle=0; bool Charging=false, Innocent=false, FixedCamera=false;
 void Forward(float V); void Right(float V); void Turn(float V); void Look(float V);
 void Press(); void Release(); void Hop(); void Retry(); void Pause(); void ToggleHUD(); void ToggleCamera(); void Tutorial();
 void ResetCat(); void SetMutation(bool On);
};
UCLASS()
class BADCATLAB_API ALabGameMode : public AGameModeBase {
 GENERATED_BODY()
public:
 ALabGameMode(); virtual void BeginPlay() override; virtual void Tick(float Dt) override;
 UPROPERTY() TArray<ALabProp*> Props;
 TMap<int32,FLabChain> Chains;
 int32 NextChain=1,Stage=0,Score=0,Best=0,Current=0,Pounces=3,Swipes=0,UsedPounces=0;
 float Remaining=45; bool Learned=false,Challenge=false,HideHUD=false;
 FString Quip; double QuipUntil=0;
 bool Playing() const {return Stage==0 || Stage==2;}
 int32 NewChain(bool Pounce); bool Active(int32 Id) const; void Affect(ALabProp* Prop,int32 Id);
 void StartRound(bool Bounce); void FinishRound(); void Retry(bool Full=false);
 void BuildRoom(); void SpawnProps(bool Bounce);
};
UCLASS()
class BADCATLAB_API ALabHUD : public AHUD {
 GENERATED_BODY()
public:
 UPROPERTY() UFont* ChineseFont = nullptr;
 virtual void DrawHUD() override;
};
