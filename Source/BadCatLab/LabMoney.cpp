#include "LabGame.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Kismet/GameplayStatics.h"

void ALabGameMode::QueueMoneyBurst(ALabProp* Prop,int32 ChainCount){
 if(!IsValid(Prop)||Prop->BaseScore<=0)return;
 const double Now=FPlatformTime::Seconds();const FVector Origin=Prop->GetActorLocation()+FVector(0,0,45);
 bool Merged=false;
 // The candy spill remains readable instead of producing 24 overlapping price labels.
 if(Prop->BaseScore<10)for(auto& B:MoneyBursts)if(B.Tiny&&Now-B.Born<.16&&FVector::DistSquared(B.Origin,Origin)<62500){B.Amount+=Prop->BaseScore;B.ChainCount=FMath::Max(B.ChainCount,ChainCount);Merged=true;break;}
 if(!Merged){if(MoneyBursts.Num()>=12)MoneyBursts.RemoveAt(0);FLabMoneyBurst B;B.Origin=Origin;B.Amount=Prop->BaseScore;B.ChainCount=ChainCount;B.Seed=Prop->PropId;B.Born=Now;B.Tiny=Prop->BaseScore<10;MoneyBursts.Add(B);}
 BillPulse=FMath::Max(BillPulse,.35f);
 // A dedicated, bounded voice lets a glass impact and a coin cue play together.
 if(Now>=CoinSoundUntil){
  if(!CoinSound)CoinSound=LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/Pixabay/CoinBurst"),nullptr,LOAD_Quiet|LOAD_NoWarn);
  if(CoinSound){if(IsValid(CoinAudio))CoinAudio->FadeOut(.045f,0);CoinAudio=UGameplayStatics::SpawnSound2D(this,CoinSound,.35f,1.f+FMath::Min(ChainCount-1,8)*.035f,0,nullptr,false,true);CoinSoundUntil=Now+.14;}
 }
}
void ALabGameMode::TickMoneyFeedback(){
 const double Now=FPlatformTime::Seconds();const float Dt=MoneyLastReal>0?FMath::Min(.1,Now-MoneyLastReal):0;MoneyLastReal=Now;
 if(!Playing())return;
 DisplayBill=FMath::FInterpTo(DisplayBill,float(Score),Dt,12);if(FMath::Abs(DisplayBill-Score)<.5f)DisplayBill=Score;
 BillPulse=FMath::Max(0.f,BillPulse-Dt*3.5f);
 MoneyBursts.RemoveAll([Now](const FLabMoneyBurst& B){return Now-B.Born>1.5;});
 for(auto& B:MoneyBursts)if(!B.Arrived&&Now-B.Born>=1.f){B.Arrived=true;BillPulse=1.f;}
}
void ALabGameMode::ClearMoneyFeedback(){
 MoneyBursts.Empty();DisplayBill=Score;BillPulse=0;MoneyLastReal=0;CoinSoundUntil=0;
 if(IsValid(CoinAudio)){CoinAudio->Stop();CoinAudio->DestroyComponent();}CoinAudio=nullptr;
}
void ALabGameMode::PreviewMoneyBurst(){
#if !UE_BUILD_SHIPPING
 if(!NoSave||!Playing())return;
 int Id=NewChain(true);const bool Stress=FParse::Param(FCommandLine::Get(),TEXT("CoinStress"));auto Targets=Props;for(auto P:Targets)if(IsValid(P)&&(Stress||P->PropId==0||P->PropId==13||P->PropId==15))P->Strike(Id,FVector(0,70,130));
#endif
}
void ALabCat::PreviewMoney(){if(auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode()))G->PreviewMoneyBurst();}
