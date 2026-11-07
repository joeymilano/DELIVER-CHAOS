#include "LabGame.h"
#include "Components/AudioComponent.h"

int32 ALabGameMode::ChooseMusicVariant(bool Tense) const {
 const bool Gameplay=Playing()||Stage==1||Stage==3;
 if(!Gameplay)return 0; // Explicitly returning to selection/title can change music.
 return Tense||(MusicVariant==2&&ChaseMustFinish)?2:1;
}

void ALabGameMode::AdvanceMusicClock(double Now){
 if(MusicClockReal>0&&MusicClockRunning)MusicPlaybackSeconds+=FMath::Max(0.,Now-MusicClockReal);
 MusicClockReal=Now;
}

void ALabGameMode::MusicFinished(UAudioComponent* Finished){
 if(Finished!=Music)return;
 AdvanceMusicClock(FPlatformTime::Seconds());MusicClockRunning=false;
 // Finished also fires on an explicit Stop. An early stop must resume the same
 // pass instead of pretending that the complete track has already been heard.
 if(MusicVariant==2&&MusicTrackDuration>0&&MusicPlaybackSeconds>=MusicTrackDuration-.75f){
  ChaseMustFinish=false;MusicPlaybackSeconds=0;++FullChaseCompletions;
  UE_LOG(LogTemp,Display,TEXT("CHASE_MUSIC_COMPLETE passes=%d duration=%.3f"),FullChaseCompletions,MusicTrackDuration);
 }
}
