#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/SaveGame.h"
#include "LabGame.generated.h"
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UPhysicalMaterial;
class UFont;
class UTexture2D;
class UAnimSequence;
class AStaticMeshActor;
class ACameraActor;
class UAudioComponent;
class USoundBase;
class ALabResident;
class UPoseableMeshComponent;
class UProceduralMeshComponent;
USTRUCT()
struct FLabScoreRow {
 GENERATED_BODY()
 UPROPERTY() int32 Money=0;
 UPROPERTY() FString Cat;
 UPROPERTY() bool Perfect=false;
};
UCLASS()
class BADCATLAB_API ULabProgress : public USaveGame {
 GENERATED_BODY()
public:
 UPROPERTY() int32 Version=1;
 UPROPERTY() int32 UnlockedMask=1;
 UPROPERTY() int32 RecipesMask=0;
 UPROPERTY() TArray<FLabScoreRow> Board;
};
USTRUCT()
struct FLabChain {
 GENERATED_BODY()
 int32 Id=0; bool Pounce=false, Resonated=false; double Last=0; TSet<int32> Members;
};
UCLASS()
class BADCATLAB_API ALabProp : public AActor {
 GENERATED_BODY()
public:
 ALabProp(); virtual void Tick(float Dt) override;
 UPROPERTY() UStaticMeshComponent* Body;
 UPROPERTY() UPhysicalMaterial* Physics;
 int32 PropId=0, ChainId=0, BaseScore=100, Bounces=0;
 bool Scored=false, Heavy=false, Mutated=false, GentlyPushed=false, DropSoundPlayed=false;
 FString ItemName=TEXT("摆件"); FVector InitialPosition;
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
 virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
 virtual void FellOutOfWorld(const UDamageType& DamageType) override;
 virtual void Landed(const FHitResult& Hit) override;
 UPROPERTY() USpringArmComponent* Arm;
 UPROPERTY() UCameraComponent* Camera;
 UPROPERTY() USceneComponent* Visual;
 UPROPERTY() TArray<UStaticMeshComponent*> Paws;
 UPROPERTY() UStaticMeshComponent* Tail;
 UPROPERTY() UStaticMeshComponent* HeroMesh;
 UPROPERTY() TArray<UStaticMeshComponent*> FusionParts;
 UPROPERTY() TArray<UAnimSequence*> Locomotion;
 int32 ActiveAnimation=INDEX_NONE;
 int32 AnimationSerial=0;
 float AnimationTime=0,ActionElapsed=0,ActionDuration=0,ContactAt=0;
 float PawReachAlpha=0,PushPhase=0,ReactionTime=0;
 FVector PawTargetWorld=FVector::ZeroVector,PendingImpulse=FVector::ZeroVector;
 TWeakObjectPtr<ALabProp> AttackTarget,PushTarget,LastPushedProp;
 int32 PendingChain=0,PushCycle=INDEX_NONE;
 bool AttackPending=false,PushContactApplied=false;
 static constexpr float PushDuration=.9f,PushContactTime=.30f,PushContactEnd=.50f;
 void CancelContact();void TickContact(float Dt);void TickCatAnimation(float Dt);
 bool PawContactPoint(ALabProp* Prop,FVector& Surface,float Slack=0) const;
 ALabProp* FindPushTarget() const;
 float StepClock=0;
 float Charge=0, ActionTime=0, Idle=0; bool Charging=false, Innocent=false, FixedCamera=false;
 FVector AimDirection=FVector::ForwardVector;
 void ConfigureKeyboardCamera();FVector ScreenForward() const;
 void MenuUp();void MenuDown();void MenuLeft();void MenuRight();
 void Forward(float V); void Right(float V); void Turn(float V); void Look(float V);
 void Press(); void Release(); void Hop(); void Retry(); void Pause(); void ToggleHUD(); void ToggleCamera(); void Tutorial();
 float CaughtTime=0,CatchGrace=0; FVector LastSafePosition=FVector(-220,0,80);
 void RecoverToSafety(); void UpdateSafety();
 void ResetCat(); void SetMutation(bool On);
 void ApplyIdentity(int32 CatIndex,bool Fused);
 void ToggleMusic();void PreviewMoney();
 void PickOne(); void PickTwo(); void PickThree(); void PickFour();
 void PushStart(); void PushStop(); void ZoomStart(); void ZoomStop(); void Meow(); void ToggleShake(); void TogglePractice();
 uint8 Traits=0; float BodyScale=1,MeshScale=1,MeshFloor=-30,ZoomTime=0,ZoomCooldown=0,MeowCooldown=0,AutoMeowCooldown=0,PushClock=0,FallSpeed=0,ImpactTime=0;
 bool Pushing=false,Shake=true,ActionPounce=false; FVector ZoomDirection; TSet<int32> ZoomHit;
 FVector AttackDirection() const; ALabProp* FindTarget(float Range,float Cone) const;
 void Impact(bool Heavy); void TickInstincts(float Dt); void NoisePulse(bool Automatic=false); void ConfirmSelection(); void OpenSelection();
};
struct FLabMoneyBurst {
 FVector Origin;int32 Amount=0,ChainCount=1,Seed=0;double Born=0;bool Tiny=false,Arrived=false;
 int CoinCount() const{return FMath::Clamp((Amount<10?3:Amount<200?4:Amount<1000?6:8)+(ChainCount>=3?1:0),3,8);}
};
UCLASS()
class BADCATLAB_API ALabGameMode : public AGameModeBase {
 GENERATED_BODY()
public:
 ALabGameMode(); virtual void BeginPlay() override; virtual void Tick(float Dt) override;
 UPROPERTY() TArray<ALabProp*> Props;
 TMap<int32,FLabChain> Chains;
 int32 NextChain=1,Stage=0,Score=0,Best=0,Current=0,Pounces=3,Swipes=0,UsedPounces=0;
 float Remaining=180; bool Learned=false,Challenge=false,HideHUD=false;
 int32 SelectedCat=0,PartnerCat=1,RecipeId=INDEX_NONE;
 UPROPERTY() TArray<AStaticMeshActor*> PresentationCats;
 FString Quip; double QuipUntil=0;
 UPROPERTY() ULabProgress* Progress;
 UPROPERTY() ALabProp* PetCamera;
 UPROPERTY() UProceduralMeshComponent* CameraVision;
 UPROPERTY() TMap<int32,AStaticMeshActor*> Discoveries;
 int32 Suspicion=0,GoodCat=0; bool CameraBroken=false,PerfectCrime=false,CandySpilled=false,Recorded=false,Practice=false,NoSave=false;
 float CameraClock=0,Exposure=0,GoodExposure=0,CaughtCooldown=0,GoodCooldown=0,CameraFlash=0,VisionClock=0;bool CameraInView=false;
 static constexpr float CameraRange=650,CameraHalfAngle=38,CameraNoticeTime=.9f;
 double SlowUntil=0; FVector WatchDirection; FString CameraCaption;
 TArray<FLabMoneyBurst> MoneyBursts;float DisplayBill=0,BillPulse=0;double MoneyLastReal=0,CoinSoundUntil=0;
 UPROPERTY() UAudioComponent* CoinAudio=nullptr;
 UPROPERTY() USoundBase* CoinSound=nullptr;
 void QueueMoneyBurst(ALabProp* Prop,int32 ChainCount);void TickMoneyFeedback();void ClearMoneyFeedback();void PreviewMoneyBurst();
 void LoadProgress(); void SaveProgress(); bool Unlocked(int32 Id) const; bool RecipeUnlocked(int32 Id) const;
 void Discover(int32 Id,FVector Position); void CollectDiscoveries(); void ResetParty(); void TickCamera(float Dt);
 bool CameraSees(FVector Position,const AActor* Target=nullptr) const; void CameraObservation(bool Bad,bool Innocent);
 void CreateCameraFootprint();void UpdateCameraFootprint();void AdvanceCameraEvidence(float Dt,bool CatSeen,bool Bad,bool Innocent);
 void DisableCamera(); void SpillCandy(FVector Origin,int32 Chain); void SpawnPartyProps(); void RadialSmash(FVector Origin,float Strength,float Radius);
 bool CatchCat(ALabCat* Cat); int32 TimesCaught=0;
 void RecordRun(); void SetPractice(); void Speak(const TCHAR* Clip); void Lab(); void ClearDiscoveries();
 UPROPERTY() UAudioComponent* OwnerVoice = nullptr;
 UPROPERTY() UAudioComponent* CatVoice = nullptr;
 double OwnerVoiceUntil=0,CatVoiceUntil=0,SoftMeowUntil=0;
 bool ReceiptGreetingPending=false;
 void ClearVoices(); void PauseVoices(bool Paused);
 uint8 CurrentTraits() const;

 bool BounceRule() const; bool ResonanceRule() const;
 int MenuCursor=0;
 FName FocusedMenuAction() const;void NavigateMenu(int DX,int DY);void ActivateMenu();
 void SelectCat(int32 Index); void ConfirmSelection(); void OpenSelection();
 void BeginFusionChoice(); void UpdatePresentation(); void SetMenuInput(bool Enabled);
 void TryResonate(ALabProp* Source,int32 Chain,FVector Direction);
 bool Playing() const {return Stage==0 || Stage==2;}
 int32 NewChain(bool Pounce); bool Active(int32 Id) const; void Affect(ALabProp* Prop,int32 Id);
 void StartRound(bool Bounce); void FinishRound(); void Retry(bool Full=false);
 UPROPERTY() TArray<ALabResident*> Residents;
 UPROPERTY() UAudioComponent* Music=nullptr;
 UPROPERTY() TArray<AActor*> TitleSet;
 UPROPERTY() UPoseableMeshComponent* TitleBody=nullptr;
 UPROPERTY() ACameraActor* TitleCamera=nullptr;
 FRotator TitleHeadRest,TitleTailRest;
 float TitleTime=0; bool MusicMuted=false,MusicGameplay=false;
 int MusicVariant=-1; double ChaseMusicUntil=0; bool EndKeyPlayed=false;
 void OpenTitle(); void ClearTitle(); void TickTitle(float Dt); void StartFromTitle(bool FreePlay=false);
 UPROPERTY() TArray<AActor*> HalloweenDecor;
 void BuildHalloweenDecor();
 void WriteSceneAudit();

 double WorldSoundUntil=0;
 void WorldSound(const TCHAR* Name,float Volume);void UpdateMusic();
 int LightingStage=4,LightingQuality=2;
 void ConfigureLighting();void BuildLightingFinish();void AddWarmPropLight(ALabProp* Prop);void TickVisualBenchmark();
 double VisualLastTime=0,VisualWarmup=0,VisualSampleSeconds=0;TArray<double> VisualFrameTimes;FString VisualLabel;bool VisualComplete=false;
 void BuildCozyCorner();void SpawnCozyProps();
 void BuildRoom(); void SpawnProps(bool Bounce);
#if !UE_BUILD_SHIPPING
 void SetupCupReview();void TickCupReview(float Dt);
 float CupReviewTime=0;int32 CupReviewStep=0;TWeakObjectPtr<ALabProp> ReviewCup;
#endif
 UPROPERTY() AActor* MemeCat = nullptr;
 UPROPERTY() ACameraActor* MemeCamera = nullptr;
 UPROPERTY() UAudioComponent* MemeAudio = nullptr;
 UPROPERTY() TArray<AActor*> MemeSet;
 UPROPERTY() UPoseableMeshComponent* MeowBody=nullptr;
 UPROPERTY() UTexture2D* MemeGifTexture=nullptr;
 FRotator MeowHeadRest,MeowJawRest;
 float MemeElapsed=0,MemeGifElapsed=0; bool MemeShowing=false,MemeGifShowing=false,MeowPlayed=false;
 void ToggleMemeGif();
 static constexpr float MemeDuration=2.2f;
 void StartMeme(); void StopMeme(); void ClearMeme(); void TickMeme(float Dt);
 static FRotator MemePose(float Time);
};
UCLASS()
class BADCATLAB_API ALabHUD : public AHUD {
 GENERATED_BODY()
public:
 UPROPERTY() UFont* ChineseFont = nullptr;
 UPROPERTY() UFont* ChineseBoldFont = nullptr;
 UPROPERTY() TArray<UTexture2D*> Portraits;
 UPROPERTY() UTexture2D* TitleMark=nullptr;
 virtual void DrawHUD() override;
 virtual void NotifyHitBoxClick(FName BoxName) override;
 int32 LastStage=INDEX_NONE; double StageEnteredAt=0, CaptionVisibleUntil=0; FString LastCaption;
};
