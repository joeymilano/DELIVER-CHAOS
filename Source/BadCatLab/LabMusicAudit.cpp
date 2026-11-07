#include "LabGame.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Sound/SoundBase.h"
#include "GenericPlatform/GenericPlatformMisc.h"

// Explicit opt-in output capture. Does not run or change the UI in normal play.
void ALabGameMode::TickMusicAudit(){
 if(NoSave&&FParse::Param(FCommandLine::Get(),TEXT("ChaseMusicAudit"))){TickFullChaseAudit();return;}
 if(!NoSave||!FParse::Param(FCommandLine::Get(),TEXT("MusicAudit")))return;
 const double Now=FPlatformTime::Seconds();
 if(MusicAuditNext==0){MusicAuditNext=Now+2;return;}
 if(Now<MusicAuditNext)return;
 const TCHAR* Names[]={TEXT("menu"),TEXT("cancan"),TEXT("muted"),TEXT("resumed"),TEXT("chase"),TEXT("held-chase")};
 FString Folder=FPaths::ProjectSavedDir()/TEXT("MusicAudit");FParse::Value(FCommandLine::Get(),TEXT("MusicAuditDir="),Folder);
 if(MusicAuditPhase>=0&&MusicAuditPhase<6){
  UAudioMixerBlueprintLibrary::StopRecordingOutput(this,EAudioRecordingExportType::WavFile,Names[MusicAuditPhase],Folder);
  UE_LOG(LogTemp,Display,TEXT("MUSIC_AUDIT captured %s, playing=%d gain=%.3f"),Names[MusicAuditPhase],IsValid(Music)&&Music->IsPlaying(),IsValid(Music)?Music->VolumeMultiplier:0);
 }
 ++MusicAuditPhase;
 if(MusicAuditPhase==6){MusicAuditNext=Now+2;return;}
 if(MusicAuditPhase>6){FPlatformMisc::RequestExit(false);return;}
 if(MusicAuditPhase==1){Practice=true;StartRound(false);}
 if(MusicAuditPhase==2)MusicMuted=true;
 if(MusicAuditPhase==3){MusicMuted=false;if(IsValid(Music))Music->Stop();}
 if(MusicAuditPhase==4)ChaseMusicUntil=GetWorld()->GetTimeSeconds()+4;
 if(MusicAuditPhase==5)ChaseMusicUntil=0;
 UAudioMixerBlueprintLibrary::StartRecordingOutput(this,5);
 MusicAuditNext=Now+4;
}

void ALabGameMode::TickFullChaseAudit(){
 if(!NoSave)return;
 const double Now=FPlatformTime::Seconds();
 FString Folder=FPaths::ProjectSavedDir()/TEXT("FullChaseAudit");FParse::Value(FCommandLine::Get(),TEXT("MusicAuditDir="),Folder);
 if(ChaseAuditStep==4){if(Now>=ChaseAuditNextSample)FPlatformMisc::RequestExit(false);return;}
 if(ChaseAuditStart==0){
  ChaseAuditStart=Now;Practice=true;StartRound(false);ChaseMusicUntil=GetWorld()->GetTimeSeconds()+1;
  IFileManager::Get().MakeDirectory(*Folder,true);
  ChaseAuditRows=TEXT("elapsed,variant,position,playing,paused,full_pass_required,completed,stage\n");
  UAudioMixerBlueprintLibrary::StartRecordingOutput(this,145);ChaseAuditStep=1;
 }
 const double Elapsed=Now-ChaseAuditStart;
 if(Now>=ChaseAuditNextSample){
  ChaseAuditRows+=FString::Printf(TEXT("%.3f,%d,%.3f,%d,%d,%d,%d,%d\n"),Elapsed,MusicVariant,MusicPlaybackSeconds,IsValid(Music)&&Music->IsPlaying(),IsValid(Music)&&Music->bIsPaused,ChaseMustFinish,FullChaseCompletions,Stage);
  ChaseAuditNextSample=Now+.25;
 }
 if(ChaseAuditStep==1&&Elapsed>=4){ChaseMusicUntil=0;ChaseAuditStep=2;}
 if(ChaseAuditStep==2&&Elapsed>=12){FinishRound();ChaseAuditStep=3;}
 auto Track=LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/Pixabay/Chase"));const float Duration=Track?Track->GetDuration():127.27f;
 const bool Done=FullChaseCompletions>0&&MusicVariant==1&&Elapsed>=Duration+5;
 if(Done||Elapsed>Duration+15){
  UAudioMixerBlueprintLibrary::StopRecordingOutput(this,EAudioRecordingExportType::WavFile,TEXT("full-chase"),Folder);
  FFileHelper::SaveStringToFile(ChaseAuditRows,*(Folder/TEXT("timeline.csv")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
  const FString Report=FString::Printf(TEXT("{\"complete\":%s,\"source_duration\":%.4f,\"recorded_elapsed\":%.4f,\"full_passes\":%d,\"final_variant\":%d,\"threat_duration\":1,\"round_finished_at\":12}"),Done?TEXT("true"):TEXT("false"),Duration,Elapsed,FullChaseCompletions,MusicVariant);
  FFileHelper::SaveStringToFile(Report,*(Folder/TEXT("result.json")));ChaseAuditStep=4;ChaseAuditNextSample=Now+2;
 }
}
