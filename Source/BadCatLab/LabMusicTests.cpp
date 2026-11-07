#if WITH_DEV_AUTOMATION_TESTS
#include "LabGame.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AutomationTest.h"
#include "Sound/SoundWave.h"

namespace {
constexpr auto Flags=EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter;
struct FMusicWorld {
 UWorld* W;ALabGameMode* G;
 FMusicWorld(){W=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(W);W->SetGameInstance(NewObject<UGameInstance>(W));W->GetWorldSettings()->DefaultGameMode=ALabGameMode::StaticClass();W->SetGameMode(FURL());G=CastChecked<ALabGameMode>(W->GetAuthGameMode());G->NoSave=true;G->Progress=NewObject<ULabProgress>(G);}
 ~FMusicWorld(){GEngine->DestroyWorldContext(W);W->DestroyWorld(false);}
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabChaseTransitionsTest,"BadCatLab.Music.FullTrackSurvivesChaseAndRoundEnd",Flags)
bool FLabChaseTransitionsTest::RunTest(const FString&){
 FMusicWorld F;auto G=F.G;G->MusicVariant=2;G->ChaseMustFinish=true;
 TestEqual(TEXT("Ending pursuit cannot cut the music"),G->ChooseMusicVariant(false),2);
 G->StartRound(false);TestEqual(TEXT("Retry retains the current full pass"),G->ChooseMusicVariant(false),2);
 G->FinishRound();TestEqual(TEXT("Receipt does not interrupt the climax"),G->ChooseMusicVariant(false),2);
 TestTrue(TEXT("Ending meow is active during this regression case"),G->MemeShowing);
 G->Stage=4;TestEqual(TEXT("Explicitly returning to cat selection can change track"),G->ChooseMusicVariant(false),0);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabChaseCompletionTest,"BadCatLab.Music.OnlyRealCompletionReleasesThePass",Flags)
bool FLabChaseCompletionTest::RunTest(const FString&){
 FMusicWorld F;auto G=F.G;auto Voice=NewObject<UAudioComponent>(G);G->Music=Voice;G->MusicVariant=2;G->ChaseMustFinish=true;G->MusicTrackDuration=127.27f;G->MusicPlaybackSeconds=50;
 G->MusicFinished(Voice);TestTrue(TEXT("Early audio stop is not full playback"),G->ChaseMustFinish);TestEqual(TEXT("Interrupted voice retains resume point"),G->MusicPlaybackSeconds,50.f);
 auto Old=NewObject<UAudioComponent>(G);G->MusicFinished(Old);TestEqual(TEXT("Retired voice cannot complete current song"),G->FullChaseCompletions,0);
 G->MusicPlaybackSeconds=127.27f;G->MusicFinished(Voice);TestFalse(TEXT("Actual end unlocks the next track"),G->ChaseMustFinish);
 TestEqual(TEXT("CanCan returns after complete song"),G->ChooseMusicVariant(false),1);TestEqual(TEXT("Continuing chase can request another complete pass"),G->ChooseMusicVariant(true),2);
 G->MusicClockReal=100;G->MusicClockRunning=true;G->AdvanceMusicClock(110);TestEqual(TEXT("Music advances in real audio time"),G->MusicPlaybackSeconds,10.f);
 G->MusicClockRunning=false;G->AdvanceMusicClock(170);TestEqual(TEXT("Muted pause does not consume the song"),G->MusicPlaybackSeconds,10.f);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabChaseAssetTest,"BadCatLab.Music.ChaseAssetHasAnObservableEnd",Flags)
bool FLabChaseAssetTest::RunTest(const FString&){
 auto Track=LoadObject<USoundWave>(nullptr,TEXT("/Game/Audio/Pixabay/Chase"));if(!TestNotNull(TEXT("Pursuit recording exists"),Track))return false;
 TestFalse(TEXT("Chase is a full non-looping pass with an end event"),Track->bLooping);
 TestTrue(TEXT("Original complete recording is retained"),Track->GetDuration()>120&&Track->GetDuration()<130);
 TestEqual(TEXT("Inline audio remains available throughout the song"),Track->GetLoadingBehavior(),ESoundWaveLoadingBehavior::ForceInline);
 return true;
}
#endif
