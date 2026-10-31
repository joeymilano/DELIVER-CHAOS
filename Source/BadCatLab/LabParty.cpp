#include "LabGame.h"
#include "LabCatalog.h"
#include "LabResident.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
static ALabGameMode* Party(UWorld* W){return Cast<ALabGameMode>(W->GetAuthGameMode());}
static void Colorize(UStaticMeshComponent* C,FLinearColor Color){auto M=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/M_Palette"));if(M){auto D=UMaterialInstanceDynamic::Create(M,C);D->SetVectorParameterValue(TEXT("Color"),Color);C->SetMaterial(0,D);}}
FVector ALabCat::AttackDirection() const{return FRotator(0,GetControlRotation().Yaw,0).Vector();}
ALabProp* ALabCat::FindTarget(float Range,float Cone) const{
 auto G=Party(GetWorld());if(!G)return nullptr;ALabProp* Best=nullptr;float BestValue=MAX_flt;const FVector Origin=GetActorLocation()+FVector(0,0,18),Aim=AttackDirection();
 for(auto P:G->Props){if(!IsValid(P)||P->IsHidden()||(P==G->PetCamera&&G->CameraBroken))continue;FVector D=P->GetActorLocation()-Origin;float Dist=D.Size();if(Dist>Range*FMath::Sqrt(BodyScale)||FVector::DotProduct(D.GetSafeNormal2D(),Aim)<Cone)continue;
 FHitResult H;FCollisionQueryParams Q;Q.AddIgnoredActor(this);bool Blocked=GetWorld()->LineTraceSingleByChannel(H,Origin,P->GetActorLocation(),ECC_Visibility,Q);if(Blocked&&H.GetActor()!=P)continue;
 float Value=Dist*(P->Scored?1.3f:1.f);if(Value<BestValue){Best=P;BestValue=Value;}}
 return Best;
}
void ALabCat::PushStart(){if(auto G=Party(GetWorld());G&&G->Playing()&&CaughtTime<=0){Pushing=true;Innocent=false;Charging=false;PushClock=0;Idle=0;}}
void ALabCat::PushStop(){Pushing=false;}
void ALabCat::ZoomStart(){if(auto G=Party(GetWorld());G&&G->Playing()&&CaughtTime<=0&&ZoomCooldown<=0){ZoomTime=(Traits&CatTrait::Giant)&&(Traits&CatTrait::Wild)?1.8f:1.1f;ZoomCooldown=2.1f;ZoomDirection=AttackDirection();ZoomHit.Empty();Innocent=false;Charging=Pushing=false;Idle=0;}}
void ALabCat::ZoomStop(){if(!(Traits&CatTrait::Wild))ZoomTime=FMath::Min(ZoomTime,.15f);}
void ALabCat::ToggleShake(){Shake=!Shake;}
void ALabCat::Meow(){NoisePulse(false);}
void ALabCat::NoisePulse(bool Automatic){
 auto G=Party(GetWorld());if(!G||!G->Playing()||CaughtTime>0||!(Traits&CatTrait::Loud)||(Automatic?AutoMeowCooldown>0:MeowCooldown>0))return;MeowCooldown=1.4f;AutoMeowCooldown=.2f;Innocent=false;Idle=0;ActionTime=.2f;
 const int Chain=G->NewChain(false);const FVector Origin=GetActorLocation(),Dir=Automatic?ZoomDirection:AttackDirection();auto Copy=G->Props;int Count=0;
 for(auto P:Copy){if(!IsValid(P)||(P->Heavy&&!(Traits&CatTrait::Giant)))continue;FVector D=P->GetActorLocation()-Origin;if(D.Size()>230*BodyScale||FVector::DotProduct(D.GetSafeNormal2D(),Dir)<.45f)continue;P->Strike(Chain,D.GetSafeNormal()*650+FVector(0,0,160));if(++Count>=8)break;}
 DrawDebugCone(GetWorld(),Origin,Dir,230*BodyScale,.8f,.5f,20,FColor(90,210,230),false,.35f,0,3);G->Quip=TEXT("听见了吧。");G->QuipUntil=GetWorld()->GetTimeSeconds()+1.5;G->Speak(TEXT("Meow"));
}
void ALabCat::Landed(const FHitResult& Hit){Super::Landed(Hit);auto G=Party(GetWorld());if(G&&G->Playing())G->WorldSound(TEXT("Landing"),.3f);if(G&&G->Playing()&&(Traits&CatTrait::Heavy)&&FallSpeed>230){G->RadialSmash(GetActorLocation(),FMath::Clamp(FallSpeed,350.f,1000.f)*BodyScale,160*BodyScale);Impact(true);}FallSpeed=0;}
void ALabCat::Impact(bool Heavy){auto G=Party(GetWorld());if(!G)return;ImpactTime=.13f;G->Speak(Heavy?TEXT("Thud"):TEXT("Clink"));G->SlowUntil=FPlatformTime::Seconds()+(Heavy?.18:.05);UGameplayStatics::SetGlobalTimeDilation(GetWorld(),Heavy?.35f:.08f);}
void ALabCat::TickInstincts(float Dt){
 auto G=Party(GetWorld());if(!G||!G->Playing()||CaughtTime>0)return;
 ZoomCooldown=FMath::Max(0.f,ZoomCooldown-Dt);MeowCooldown=FMath::Max(0.f,MeowCooldown-Dt);AutoMeowCooldown=FMath::Max(0.f,AutoMeowCooldown-Dt);
 if(GetCharacterMovement()->IsFalling())FallSpeed=FMath::Max(FallSpeed,-GetVelocity().Z);
 if(Pushing){PushClock-=Dt;if(PushClock<=0){PushClock=.12f;if(auto P=FindTarget(140,.15f)){int Id=G->Active(P->ChainId)?P->ChainId:G->NewChain(false);P->ChainId=Id;G->Chains[Id].Last=GetWorld()->GetTimeSeconds();FVector V=P->Body->GetPhysicsLinearVelocity();const FVector D=AttackDirection();P->Body->SetPhysicsLinearVelocity(FVector(D.X*55,D.Y*55,V.Z));ActionTime=.1f;}}}
 GetCharacterMovement()->MaxWalkSpeed=ZoomTime>0?950:LabCatalog::Cat(G->SelectedCat).Speed;
 GetCharacterMovement()->RotationRate=FRotator(0,ZoomTime>0?180:800,0);
 if(ZoomTime>0){ZoomTime-=Dt;AddMovementInput(ZoomDirection,1);SetActorRotation(ZoomDirection.Rotation());auto Copy=G->Props;
  for(auto P:Copy){if(!IsValid(P)||ZoomHit.Contains(P->PropId)||FVector::Dist(P->GetActorLocation(),GetActorLocation())>75*BodyScale)continue;
   ZoomHit.Add(P->PropId);P->Strike(G->NewChain(false),ZoomDirection*(Traits&CatTrait::Giant?1600:950)+FVector(0,0,180));Impact(P->Heavy);
   if(Traits&CatTrait::Loud)NoisePulse(true);
   if((Traits&CatTrait::Wild)&&!(Traits&CatTrait::Giant)){ZoomDirection=(-ZoomDirection).RotateAngleAxis(FMath::FRandRange(-35.f,35.f),FVector::UpVector);LaunchCharacter(ZoomDirection*420+FVector(0,0,160),true,true);}break;
  }
 }
}
void ALabGameMode::LoadProgress(){
 NoSave=FParse::Param(FCommandLine::Get(),TEXT("NoSave"));Progress=NoSave?nullptr:Cast<ULabProgress>(UGameplayStatics::LoadGameFromSlot(TEXT("DeliverChaos_Party_v1"),0));if(!Progress)Progress=NewObject<ULabProgress>(this);
 Progress->UnlockedMask=(Progress->UnlockedMask&15)|1;Progress->RecipesMask&=63;Progress->Board.Sort([](const FLabScoreRow& A,const FLabScoreRow& B){return A.Money>B.Money;});if(Progress->Board.Num()>5)Progress->Board.SetNum(5);
}
void ALabGameMode::SaveProgress(){if(Progress&&!NoSave)UGameplayStatics::SaveGameToSlot(Progress,TEXT("DeliverChaos_Party_v1"),0);}
bool ALabGameMode::Unlocked(int32 Id) const{return Id>=0&&Id<4&&((Progress?Progress->UnlockedMask:1)&(1<<Id));}
bool ALabGameMode::RecipeUnlocked(int32 Id) const{if(Id<0||Id>=6)return false;if(!LabCatalog::Recipe(Id).Hidden)return true;int Count=Progress?FMath::CountBits((uint32)(Progress->RecipesMask&15)):0;return Id==4?Count>=2:Progress&&Progress->UnlockedMask==15&&FMath::CountBits((uint32)(Progress->RecipesMask&31))>=3;}
uint8 ALabGameMode::CurrentTraits() const{return RecipeId>=0?LabCatalog::Recipe(RecipeId).Traits:LabCatalog::Cat(SelectedCat).Traits;}
void ALabGameMode::ClearDiscoveries(){for(auto& Pair:Discoveries)if(IsValid(Pair.Value))Pair.Value->Destroy();Discoveries.Empty();}
void ALabGameMode::Discover(int32 Id,FVector Pos){
 if(Unlocked(Id)||Discoveries.Contains(Id))return;auto M=LoadObject<UStaticMesh>(nullptr,LabCatalog::Cat(Id).Mesh);if(!M)return;auto A=GetWorld()->SpawnActor<AStaticMeshActor>(Pos,FRotator(0,-90,0));auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(M);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetWorldScale3D(FVector(35.f/FMath::Max(1.f,M->GetBoundingBox().GetSize().Z)));Discoveries.Add(Id,A);Quip=TEXT("这里还藏了一只！靠近它，就能带回去。");QuipUntil=GetWorld()->GetTimeSeconds()+4;
}
void ALabGameMode::CollectDiscoveries(){auto Cat=UGameplayStatics::GetPlayerPawn(GetWorld(),0);if(!Cat||!Progress)return;TArray<int32> Done;
 for(auto& Pair:Discoveries){auto A=Pair.Value;if(!IsValid(A))continue;A->AddActorWorldRotation(FRotator(0,60*GetWorld()->GetDeltaSeconds(),0));DrawDebugCircle(GetWorld(),A->GetActorLocation(),32,20,FColor::Orange,false,0,0,2,FVector::ForwardVector,FVector::RightVector,false);
 if(FVector::Dist(A->GetActorLocation(),Cat->GetActorLocation())<125){Progress->UnlockedMask|=1<<Pair.Key;Quip=FString::Printf(TEXT("%s 跟你走了。下次可以选它，也可以合成。"),LabCatalog::Cat(Pair.Key).Name);QuipUntil=GetWorld()->GetTimeSeconds()+4;A->Destroy();Done.Add(Pair.Key);}}
 if(Done.Num()){for(int I:Done)Discoveries.Remove(I);SaveProgress();}
}
void ALabGameMode::ResetParty(){ClearMoneyFeedback();TimesCaught=0;ClearVoices();Suspicion=GoodCat=0;CameraBroken=PerfectCrime=CandySpilled=false;Exposure=CaughtCooldown=GoodCooldown=CameraFlash=CameraClock=0;CameraCaption=TEXT("");PetCamera=nullptr;CameraVision=nullptr;}
void ALabGameMode::SpawnPartyProps(){
 auto Spawn=[&](int Id,FVector Pos,FVector Scale,FLinearColor Color,const TCHAR* Name,int Price,bool Ball=false){auto P=GetWorld()->SpawnActor<ALabProp>(Pos,FRotator::ZeroRotator);P->Configure(Id,Scale,Color,Ball,false,false);P->ItemName=Name;P->BaseScore=Price;Props.Add(P);return P;};
 auto Box=Spawn(18,FVector(-140,150,23),FVector(.6,.55,.45),FLinearColor(.48,.28,.1),TEXT("快递纸箱"),12);Box->Body->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Art/Party/SM_Parcel")));
 auto Bowl=Spawn(19,FVector(80,115,22),FVector(.65,.65,.20),FLinearColor(.3,.03,.48),TEXT("糖果碗"),129);if(auto M=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Art/Party/SM_CandyBowl")))Bowl->Body->SetStaticMesh(M);
 for(int I=0;I<6;I++){auto Candy=GetWorld()->SpawnActor<AStaticMeshActor>(FVector(65+(I%3)*12,105+(I/3)*14,35),FRotator::ZeroRotator);auto C=Candy->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere")));C->SetWorldScale3D(FVector(.09,.06,.06));C->SetCollisionEnabled(ECollisionEnabled::NoCollision);Colorize(C,I%2?FLinearColor(1,.2,.03):FLinearColor(.4,.05,.6));Candy->AttachToActor(Bowl,FAttachmentTransformRules::KeepWorldTransform);PresentationCats.Add(Candy);}
 for(int I=0;I<3;I++){auto P=Spawn(20+I,FVector(160,80,20+I*33),FVector(.38,.38,.32),FLinearColor(1,.16,.01),TEXT("南瓜灯"),79,true);P->Body->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Art/Party/SM_Pumpkin")));P->SetActorRotation(FRotator(0,90,0));if(I==1)AddWarmPropLight(P);if(I==2)P->ItemName=TEXT("藏猫南瓜灯");}
 // Ascending perches create a reliable high route for orange-cat landings.
 PetCamera=Spawn(30,FVector(310,-245,115),FVector(.3,.3,.30),FLinearColor(.9,.87,.8),TEXT("宠物摄像头"),399,true);PetCamera->Body->SetSimulatePhysics(false);
 // Surveillance uses an invisible query; no world-space cone obscures the room.

 auto Lens=NewObject<UStaticMeshComponent>(PetCamera);Lens->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere")));Lens->SetupAttachment(PetCamera->Body);Lens->RegisterComponent();Lens->SetRelativeLocation(FVector(45,0,0));Lens->SetRelativeScale3D(FVector(.25,.55,.55));Lens->SetCollisionEnabled(ECollisionEnabled::NoCollision);Colorize(Lens,FLinearColor(.005,.015,.02));
 // Generated shell keeps the existing simple collision body and surveillance origin.
 if(auto ShellMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Art/V2Fun/PetCamera/SM_PetCamera"))){
  PetCamera->Body->SetVisibility(false,false);PetCamera->Body->SetCastShadow(false);Lens->SetVisibility(false);
  auto Shell=NewObject<UStaticMeshComponent>(PetCamera);Shell->SetupAttachment(PetCamera->Body);Shell->SetStaticMesh(ShellMesh);Shell->RegisterComponent();Shell->SetRelativeLocation(FVector(0,0,-50));Shell->SetRelativeRotation(FRotator(0,-90,0));Shell->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 }
 auto Cake=Spawn(23,FVector(65,-40,45),FVector(.22,.22,.22),FLinearColor(.9,.8,.6),TEXT("万圣节蛋糕"),329);
 if(auto CakeMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Art/V2Fun/HalloweenCakeV2/SM_HalloweenCakeV2"))){Cake->Body->SetSimulatePhysics(false);Cake->Body->SetStaticMesh(CakeMesh);Cake->Body->EmptyOverrideMaterials();Cake->Body->SetSimulatePhysics(true);Cake->Body->SetMassOverrideInKg(NAME_None,1.5f);}
 // Camera mount on the shelf: static decoration, recycled with the round.
 auto Stand=GetWorld()->SpawnActor<AStaticMeshActor>(FVector(310,-245,80),FRotator::ZeroRotator);auto C=Stand->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder")));C->SetWorldScale3D(FVector(.10,.10,.45));C->SetCollisionProfileName(TEXT("NoCollision"));Colorize(C,FLinearColor(.05,.05,.05));PresentationCats.Add(Stand);
}
void ALabGameMode::SpillCandy(FVector Origin,int32 Chain){
 if(CandySpilled)return;CandySpilled=true;for(auto P:Props)if(IsValid(P)&&P->PropId==19){TArray<AActor*> Attached;P->GetAttachedActors(Attached);for(auto A:Attached)if(IsValid(A))A->SetActorHiddenInGame(true);}Quip=TEXT("糖果撒了。小东西开始满屋乱弹！");QuipUntil=GetWorld()->GetTimeSeconds()+3;
 for(auto P:Props)if(IsValid(P)&&!P->Heavy&&P!=PetCamera){P->Mutated=true;P->Physics->Restitution=.82f;P->Body->SetPhysMaterialOverride(P->Physics);}
 for(int I=0;I<24;I++){FVector D=FVector(FMath::Cos(I*2.4f),FMath::Sin(I*2.4f),.8f);auto P=GetWorld()->SpawnActor<ALabProp>(Origin+D*(20+I%3*7),FRotator::ZeroRotator);P->Configure(100+I,FVector(.07,.05,.05),I%2?FLinearColor(1,.2,.02):FLinearColor(.5,.05,.7),true,false,true);P->Body->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Art/Party/SM_Candy")));P->ItemName=TEXT("糖果");P->BaseScore=2;P->ChainId=Chain;P->Body->SetMassOverrideInKg(NAME_None,.08f);P->Body->AddImpulse(D*260,NAME_None,true);Props.Add(P);}
}
void ALabGameMode::RadialSmash(FVector Origin,float Strength,float Radius){int Id=NewChain(false);auto Copy=Props;for(auto P:Copy)if(IsValid(P)&&FVector::Dist(P->GetActorLocation(),Origin)<Radius){FVector D=(P->GetActorLocation()-Origin).GetSafeNormal2D();P->Strike(Id,D*Strength+FVector(0,0,Strength*.5f));}DrawDebugCircle(GetWorld(),Origin,Radius,40,FColor::Orange,false,.4f,0,5,FVector::ForwardVector,FVector::RightVector,false);Quip=TEXT("我就坐一下。");QuipUntil=GetWorld()->GetTimeSeconds()+2;}
bool ALabGameMode::CameraSees(FVector Position,const AActor* Target) const{
 if(CameraBroken||!IsValid(PetCamera))return false;const FVector Origin=PetCamera->GetActorLocation()+WatchDirection*20,D=Position-Origin;if(D.Size()>700||FVector::DotProduct(D.GetSafeNormal(),WatchDirection)<FMath::Cos(FMath::DegreesToRadians(38.f)))return false;
 FHitResult H;FCollisionQueryParams Q;Q.AddIgnoredActor(PetCamera);bool Blocked=GetWorld()->LineTraceSingleByChannel(H,Origin,Position,ECC_Visibility,Q);return !Blocked||H.GetActor()==Target;
}
void ALabGameMode::CameraObservation(bool Bad,bool IsInnocent){
 if(CameraBroken)return;
 if(IsInnocent&&GoodCooldown<=0&&GoodCat<3){GoodCat++;Suspicion=FMath::Max(0,Suspicion-1);GoodCooldown=8;CameraCaption=TEXT("装乖成功。");Speak(TEXT("OwnerGood"));}
 else if(Bad&&!IsInnocent&&CaughtCooldown<=0){Suspicion=FMath::Min(5,Suspicion+1);CaughtCooldown=3;CameraFlash=.8f;if(!Practice)Remaining=FMath::Max(0.f,Remaining-3);CameraCaption=Practice?TEXT("爪子放下！嫌疑 +1"):TEXT("被拍到了！时间 −3 秒");Speak(TEXT("OwnerCaught"));}
}
void ALabGameMode::TickCamera(float Dt){
 CaughtCooldown=FMath::Max(0.f,CaughtCooldown-Dt);GoodCooldown=FMath::Max(0.f,GoodCooldown-Dt);CameraFlash=FMath::Max(0.f,CameraFlash-Dt);if(CameraBroken||!IsValid(PetCamera))return;CameraClock+=Dt;
 const FRotator Aim(-15,145+FMath::Sin(CameraClock*.65f)*58,0);WatchDirection=Aim.Vector();PetCamera->SetActorRotation(Aim);const FVector O=PetCamera->GetActorLocation()+WatchDirection*20;

 auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));if(!Cat)return;bool CatSeen=CameraSees(Cat->GetActorLocation(),Cat);bool Bad=false;
 for(auto P:Props)if(IsValid(P)&&P!=PetCamera&&P->Scored&&P->Body->GetPhysicsLinearVelocity().Size()>35&&CameraSees(P->GetActorLocation(),P)){Bad=true;break;}
 if(CatSeen&&(Cat->Charging||Cat->Pushing||Cat->ZoomTime>0||Cat->ActionTime>0))Bad=true;
 Exposure=(CatSeen||Bad)?Exposure+Dt:0;if(Exposure>.3f)CameraObservation(Bad,CatSeen&&Cat->Innocent);
}
void ALabGameMode::DisableCamera(){if(CameraBroken)return;CameraBroken=true;PerfectCrime=true;if(IsValid(CameraVision))CameraVision->SetVisibility(false);Speak(TEXT("CameraOff"));if(IsValid(PetCamera)){PetCamera->Body->SetSimulatePhysics(true);PetCamera->Body->AddImpulse(FVector(-300,150,80),NAME_None,true);}CameraCaption=TEXT("监控断了。刚才那段，应该没存下来。");Quip=TEXT("它看不见了。继续。");QuipUntil=GetWorld()->GetTimeSeconds()+4;}
void ALabGameMode::RecordRun(){if(Recorded||Practice||!Progress)return;Recorded=true;FLabScoreRow R;R.Money=Score;R.Cat=RecipeId>=0?LabCatalog::Recipe(RecipeId).Name:LabCatalog::Cat(SelectedCat).Name;R.Perfect=PerfectCrime;Progress->Board.Add(R);Progress->Board.StableSort([](const FLabScoreRow& A,const FLabScoreRow& B){return A.Money>B.Money;});if(Progress->Board.Num()>5)Progress->Board.SetNum(5);SaveProgress();}
void ALabGameMode::ClearVoices(){
 for(auto C:{OwnerVoice,CatVoice})if(IsValid(C)){C->Stop();C->DestroyComponent();}
 OwnerVoice=CatVoice=nullptr;OwnerVoiceUntil=CatVoiceUntil=SoftMeowUntil=0;ReceiptGreetingPending=false;
}
void ALabGameMode::PauseVoices(bool Paused){for(auto C:{OwnerVoice,CatVoice,CoinAudio})if(IsValid(C))C->SetPaused(Paused);}
void ALabGameMode::Speak(const TCHAR* Clip){
 const FString Name(Clip);const double Now=GetWorld()->GetTimeSeconds();
 const bool Owner=Name.StartsWith(TEXT("Owner")),Meow=Name.StartsWith(TEXT("Meow")),Soft=Name==TEXT("MeowSoft");
 if(Owner&&Name!=TEXT("OwnerHome")&&Now<OwnerVoiceUntil)return;
 if(Meow&&(Now<CatVoiceUntil||(Soft&&Now<SoftMeowUntil)))return;
 const FString Path=Name==TEXT("Meow")?FString(TEXT("/Game/Audio/Pixabay/MeowLoud")):Owner?FString::Printf(TEXT("/Game/Audio/ElevenLabsLocal/%s"),Clip):FString::Printf(TEXT("/Game/Audio/%s"),Clip);
 auto Sound=LoadObject<USoundBase>(nullptr,*Path,nullptr,LOAD_Quiet|LOAD_NoWarn);if(!Sound)return;
 if(Owner){
  if(IsValid(OwnerVoice)){OwnerVoice->Stop();OwnerVoice->DestroyComponent();}
  OwnerVoiceUntil=Now+Sound->GetDuration()+.3;OwnerVoice=UGameplayStatics::SpawnSound2D(GetWorld(),Sound,.78f);
 }else if(Meow){
  if(IsValid(CatVoice)){CatVoice->Stop();CatVoice->DestroyComponent();}
  const float Pitch=SelectedCat==3?.92f:SelectedCat==1?.97f:1.f;
  CatVoiceUntil=Now+Sound->GetDuration()/Pitch+.25;SoftMeowUntil=Now+10;
  CatVoice=UGameplayStatics::SpawnSound2D(GetWorld(),Sound,Soft?.5f:.72f,Pitch);
 }else UGameplayStatics::PlaySound2D(GetWorld(),Sound,.7f);
}
void ALabGameMode::SetPractice(){if(Stage==6){StartFromTitle(true);return;}if(Stage==4)Practice=!Practice;}
void ALabGameMode::Lab(){OpenSelection();}

void ALabGameMode::WorldSound(const TCHAR* Name,float Volume){
 double Now=GetWorld()->GetTimeSeconds();const FString N(Name);
 const bool Important=N==TEXT("Fusion")||N==TEXT("Key")||N==TEXT("Cash")||N==TEXT("Win");if(Now<WorldSoundUntil&&!Important)return;
 const bool Added=N==TEXT("Swipe")||N==TEXT("Landing")||N==TEXT("Glass")||N==TEXT("Fusion")||N==TEXT("Key")||N==TEXT("Cash")||N==TEXT("Win");
 auto Sound=LoadObject<USoundBase>(nullptr,*FString::Printf(TEXT("/Game/Audio/%s/%s"),Added?TEXT("Pixabay"):TEXT("World"),Name));
 if(Sound){WorldSoundUntil=Now+.1;UGameplayStatics::PlaySound2D(this,Sound,Volume);}
}
void ALabGameMode::UpdateMusic(){
 const double Now=GetWorld()->GetTimeSeconds();
 if(Playing())for(auto R:Residents)if(IsValid(R)&&R->Alert>0)ChaseMusicUntil=Now+3;
 const bool Gameplay=Playing()||Stage==1||Stage==3;
 const bool Tense=Playing()&&((!Practice&&Remaining<=20)||ChaseMusicUntil>Now);
 const int Variant=Gameplay?(Tense?2:1):0;
 if(IsValid(Music)&&MusicVariant!=Variant){Music->FadeOut(.3f,0);Music=nullptr;}
 if(!IsValid(Music)){
  const TCHAR* Path=Variant==0?TEXT("/Game/Audio/Pixabay/Menu"):Variant==2?TEXT("/Game/Audio/Pixabay/Chase"):TEXT("/Game/Audio/Gameplay/CanCanGameplay");
  if(auto Sound=LoadObject<USoundBase>(nullptr,Path)){MusicGameplay=Gameplay;MusicVariant=Variant;Music=UGameplayStatics::SpawnSound2D(this,Sound,0.f,1,0,nullptr,true,true);if(Music)Music->FadeIn(.35f,MusicMuted||MemeShowing?0.f:.32f);UE_LOG(LogTemp,Display,TEXT("BGM selected: %s"),Path);}
 }
 if(IsValid(Music))Music->SetVolumeMultiplier(MusicMuted||MemeShowing?0.f:Stage==3?.1f:Gameplay?.32f:Stage==6?.32f:.18f);
}

bool ALabGameMode::CatchCat(ALabCat* Cat){
 if(!Playing()||!IsValid(Cat)||Cat->CatchGrace>0)return false;
 UGameplayStatics::SetGlobalTimeDilation(GetWorld(),1);SlowUntil=0;TimesCaught++;
 if(!Practice)Remaining=FMath::Max(0.f,Remaining-15);
 Cat->ResetCat();Cat->CaughtTime=2;Cat->CatchGrace=6;Cat->Idle=2;
 Quip=Practice?TEXT("被抱回猫窝 · 罚坐 2 秒"):TEXT("被抱回猫窝 · −15 秒 · 罚坐 2 秒");QuipUntil=GetWorld()->GetTimeSeconds()+4;
 Speak(TEXT("OwnerCaught"));return true;
}
