#if WITH_DEV_AUTOMATION_TESTS
#include "LabGame.h"
#include "LabCatalog.h"
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
namespace {
struct FPartyWorld {
 UWorld* W; ALabGameMode* G;
 FPartyWorld(){W=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(W);W->SetGameInstance(NewObject<UGameInstance>(W));W->GetWorldSettings()->DefaultGameMode=ALabGameMode::StaticClass();W->SetGameMode(FURL());G=CastChecked<ALabGameMode>(W->GetAuthGameMode());G->NoSave=true;G->Progress=NewObject<ULabProgress>(G);}
 ~FPartyWorld(){GEngine->DestroyWorldContext(W);W->DestroyWorld(false);}
 ALabProp* Prop(int Id,int Price=100,FVector Pos=FVector(0,0,50)){auto P=W->SpawnActor<ALabProp>(Pos,FRotator::ZeroRotator);P->PropId=Id;P->BaseScore=Price;G->Props.Add(P);return P;}
};
constexpr auto Flags=EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabBillTest,"BadCatLab.Party.UniqueBill",Flags)
bool FLabBillTest::RunTest(const FString&){
 FPartyWorld F;auto G=F.G;auto A=F.Prop(1,89);auto B=F.Prop(2,4999);int C=G->NewChain(true);
 G->Affect(A,C);G->Affect(A,C);TestEqual(TEXT("Cup billed once"),G->Score,89);
 G->Affect(B,C);TestEqual(TEXT("Bill adds real prices without multiplier"),G->Score,5088);
 int Other=G->NewChain(false);G->Affect(A,Other);TestEqual(TEXT("New attack cannot rebill cup"),G->Score,5088);
 auto D=F.Prop(3);G->Chains[C].Last=-10;G->Affect(D,C);TestFalse(TEXT("Expired collision cannot create damage"),D->Scored);
 G->Stage=1;G->Affect(D,Other);TestFalse(TEXT("Fusion cannot add damage"),D->Scored);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabPounceTest,"BadCatLab.Party.UnlimitedPounce",Flags)
bool FLabPounceTest::RunTest(const FString&){
 FPartyWorld F;auto Cat=F.W->SpawnActor<ALabCat>();
 for(int I=0;I<12;I++){Cat->Press();Cat->Charge=.5f;Cat->Release();}
 TestEqual(TEXT("Twelve normal pounces retain all three supers"),F.G->Pounces,3);TestEqual(TEXT("All normal attacks accepted"),F.G->Swipes,12);
 for(int I=0;I<5;I++){Cat->Press();Cat->Charge=1.25f;Cat->Release();}
 TestEqual(TEXT("Super budget cannot go negative"),F.G->Pounces,0);TestEqual(TEXT("Only first three long charges use super"),F.G->UsedPounces,3);TestEqual(TEXT("Long charges still work after exhaustion"),F.G->Swipes,17);
 Cat->Press();Cat->Charge=.05f;Cat->Release();TestEqual(TEXT("Swipe works with no supers"),F.G->Swipes,18);
 F.G->Stage=3;Cat->Press();Cat->Release();TestEqual(TEXT("Results block attacks"),F.G->Swipes,18);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabFusionTest,"BadCatLab.Party.CollectionAndRecipes",Flags)
bool FLabFusionTest::RunTest(const FString&){
 TSet<int> Recipes;for(int A=0;A<4;A++)for(int B=0;B<4;B++){int R=LabCatalog::RecipeFor(A,B);TestEqual(TEXT("Order does not change fusion"),R,LabCatalog::RecipeFor(B,A));TestEqual(TEXT("Self fusion excluded"),R==INDEX_NONE,A==B);if(R>=0){Recipes.Add(R);TestEqual(TEXT("Fusion retains both traits"),LabCatalog::Recipe(R).Traits,uint8(LabCatalog::Cat(A).Traits|LabCatalog::Cat(B).Traits));}}
 TestEqual(TEXT("Four cats produce six unique pairs"),Recipes.Num(),6);TestEqual(TEXT("Invalid index rejected"),LabCatalog::RecipeFor(-1,3),INDEX_NONE);
 FPartyWorld F;auto G=F.G;G->OpenSelection();G->SelectCat(2);TestEqual(TEXT("Locked cat cannot be selected"),G->SelectedCat,0);
 G->Progress->UnlockedMask=15;G->SelectCat(2);G->BeginFusionChoice();G->SelectCat(2);TestEqual(TEXT("Main cat cannot be partner"),G->PartnerCat,0);
 G->SelectCat(1);G->ConfirmSelection();TestEqual(TEXT("Valid choice begins fusion"),G->Stage,1);TestEqual(TEXT("Loud plus heavy"),G->RecipeId,2);TestEqual(TEXT("Recipe recorded"),G->Progress->RecipesMask,4);
 TestFalse(TEXT("Ultimate locked before experiments"),G->RecipeUnlocked(5));G->Progress->RecipesMask=7;TestTrue(TEXT("Ultimate opens after three other recipes and four cats"),G->RecipeUnlocked(5));G->Progress->UnlockedMask=7;TestFalse(TEXT("Ultimate still needs all cats"),G->RecipeUnlocked(5));
 TestTrue(TEXT("Second hidden recipe opens after two ordinary recipes"),G->RecipeUnlocked(4));G->Stage=2;TestFalse(TEXT("Cat fusion does not automatically bounce props"),G->BounceRule());G->CandySpilled=true;TestTrue(TEXT("Candy adds bounce rule"),G->BounceRule());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabCameraTest,"BadCatLab.Party.CameraConsequences",Flags)
bool FLabCameraTest::RunTest(const FString&){
 FPartyWorld F;auto G=F.G;G->CameraObservation(true,false);TestEqual(TEXT("Caught adds suspicion"),G->Suspicion,1);TestEqual(TEXT("Caught brings owner home sooner"),G->Remaining,57.f);
 G->CameraObservation(true,false);TestEqual(TEXT("Cooldown prevents per-frame punishment"),G->Suspicion,1);
 G->CameraObservation(true,true);TestEqual(TEXT("Innocent pose wins even during falling objects"),G->GoodCat,1);TestEqual(TEXT("Innocence reduces suspicion"),G->Suspicion,0);
 G->GoodCooldown=0;G->CameraObservation(false,true);G->GoodCooldown=0;G->CameraObservation(false,true);G->GoodCooldown=0;G->CameraObservation(false,true);TestEqual(TEXT("Good-cat farming capped"),G->GoodCat,3);
 G->DisableCamera();G->CaughtCooldown=0;G->CameraObservation(true,false);TestEqual(TEXT("Broken camera cannot catch cat"),G->Suspicion,0);TestTrue(TEXT("Disabling camera grants perfect crime"),G->PerfectCrime);
 G->ResetParty();G->Practice=true;G->Remaining=60;G->CameraObservation(true,false);TestEqual(TEXT("Free mode has no time punishment"),G->Remaining,60.f);
 G->PetCamera=F.Prop(30,399);G->PetCamera->SetActorLocation(FVector(0,0,300));G->WatchDirection=FVector::ForwardVector;
 TestTrue(TEXT("Front inside cone visible"),G->CameraSees(FVector(200,0,300)));TestFalse(TEXT("Behind cone invisible"),G->CameraSees(FVector(-200,0,300)));TestFalse(TEXT("Out of range invisible"),G->CameraSees(FVector(900,0,300)));
 auto Wall=F.Prop(80,0,FVector(100,0,300));Wall->Body->SetCollisionProfileName(TEXT("BlockAll"));TestFalse(TEXT("Solid object occludes line of sight"),G->CameraSees(FVector(200,0,300)));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabSaveTest,"BadCatLab.Party.LocalProgress",Flags)
bool FLabSaveTest::RunTest(const FString&){
 FPartyWorld F;auto G=F.G;G->Progress->UnlockedMask=15;G->Progress->RecipesMask=7;
 for(int I=0;I<9;I++){G->Recorded=false;G->Score=I*100;G->RecordRun();G->RecordRun();}
 TestEqual(TEXT("Leaderboard contains top five only"),G->Progress->Board.Num(),5);TestEqual(TEXT("Highest loss first"),G->Progress->Board[0].Money,800);TestEqual(TEXT("Duplicate finish does not add entry"),G->Progress->Board[4].Money,400);
 G->Practice=true;G->Recorded=false;G->Score=999999;G->RecordRun();TestEqual(TEXT("Practice excluded from leaderboard"),G->Progress->Board[0].Money,800);
 TArray<uint8> Bytes;TestTrue(TEXT("Progress serializes"),UGameplayStatics::SaveGameToMemory(G->Progress,Bytes));auto Loaded=Cast<ULabProgress>(UGameplayStatics::LoadGameFromMemory(Bytes));TestNotNull(TEXT("Progress loads"),Loaded);if(Loaded){TestEqual(TEXT("Cat collection survives"),Loaded->UnlockedMask,15);TestEqual(TEXT("Recipes survive"),Loaded->RecipesMask,7);TestEqual(TEXT("Board survives"),Loaded->Board[0].Money,800);}
 const FString Slot=TEXT("DeliverChaos_Automation_")+FGuid::NewGuid().ToString();TestTrue(TEXT("Progress writes to disk"),UGameplayStatics::SaveGameToSlot(G->Progress,Slot,0));auto Disk=Cast<ULabProgress>(UGameplayStatics::LoadGameFromSlot(Slot,0));TestNotNull(TEXT("Progress reads back from disk"),Disk);if(Disk)TestEqual(TEXT("Disk leaderboard survives"),Disk->Board[0].Money,800);UGameplayStatics::DeleteGameInSlot(Slot,0);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabResetTest,"BadCatLab.Party.TwentyRoundReset",Flags)
bool FLabResetTest::RunTest(const FString&){
 FPartyWorld F;auto G=F.G;
 for(int Round=0;Round<20;Round++){
  G->RecipeId=Round%2?2:1;G->StartRound(true);TestEqual(TEXT("Each round starts with 25 props"),G->Props.Num(),25);
  ALabProp* Bowl=nullptr;for(auto P:G->Props)if(P->PropId==19)Bowl=P;
  TestNotNull(TEXT("Candy bowl exists"),Bowl);if(!Bowl)return false;int C=G->NewChain(true);G->Affect(Bowl,C);TestEqual(TEXT("Bowl releases exactly 24 candies"),G->Props.Num(),49);G->Affect(Bowl,G->NewChain(false));TestEqual(TEXT("Bowl cannot duplicate candies"),G->Props.Num(),49);
  G->DisableCamera();G->Suspicion=4;G->OpenSelection();TestEqual(TEXT("Reset clears all props"),G->Props.Num(),0);TestEqual(TEXT("Reset clears all presentation actors"),G->PresentationCats.Num(),0);TestEqual(TEXT("Reset clears money"),G->Score,0);TestEqual(TEXT("Reset clears chains"),G->Chains.Num(),0);TestEqual(TEXT("Reset clears suspicion"),G->Suspicion,0);TestFalse(TEXT("Reset restores camera"),G->CameraBroken);TestFalse(TEXT("Reset clears candy rule"),G->CandySpilled);TestEqual(TEXT("Reset restores time scale"),UGameplayStatics::GetGlobalTimeDilation(F.W),1.f);
 }
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabPulseTest,"BadCatLab.Party.BoundedResonance",Flags)
bool FLabPulseTest::RunTest(const FString&){
 FPartyWorld F;auto G=F.G;G->RecipeId=1;for(int I=0;I<7;I++){auto P=F.Prop(I,100,FVector(I*15,0,50));P->Heavy=I==1;P->Body->SetSimulatePhysics(true);}auto Source=G->Props[0];int C=G->NewChain(true);G->Affect(Source,C);G->TryResonate(Source,C,FVector::ForwardVector);
 TestEqual(TEXT("One source plus at most three resonating targets"),G->Chains[C].Members.Num(),4);TestFalse(TEXT("Heavy furniture excluded"),G->Props[1]->Scored);G->TryResonate(Source,C,FVector::ForwardVector);TestEqual(TEXT("Resonance fires only once per chain"),G->Chains[C].Members.Num(),4);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabSoftPushTest,"BadCatLab.Party.SlowPushDelaysBill",Flags)
bool FLabSoftPushTest::RunTest(const FString&){
 FPartyWorld F;auto P=F.Prop(0,89,FVector(0,0,60));P->InitialPosition=P->GetActorLocation();P->ChainId=F.G->NewChain(false);
 P->Tick(.016f);TestEqual(TEXT("Touching cup does not bill it"),F.G->Score,0);
 P->SetActorLocation(FVector(15,0,60));P->Tick(.016f);TestEqual(TEXT("Small tabletop nudge does not bill it"),F.G->Score,0);
 P->SetActorLocation(FVector(25,0,45));P->Tick(.016f);TestEqual(TEXT("Cup falling off edge adds its price"),F.G->Score,89);P->Tick(.016f);TestEqual(TEXT("Falling cup billed only once"),F.G->Score,89);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabMemeTest,"BadCatLab.Party.MemeCelebrationLifecycle",Flags)
bool FLabMemeTest::RunTest(const FString&){
 TestTrue(TEXT("Starts facing camera"),ALabGameMode::MemePose(0).IsNearlyZero());
 TestFalse(TEXT("Spin burst changes yaw"),ALabGameMode::MemePose(.9f).IsNearlyZero());
 TestTrue(TEXT("First deadpan pause faces front"),ALabGameMode::MemePose(1.45f).IsNearlyZero(.01f));
 TestTrue(TEXT("Finishes with complete turns"),ALabGameMode::MemePose(4.8f).IsNearlyZero(.01f));
 TestEqual(TEXT("No camera-like roll"),ALabGameMode::MemePose(.9f).Roll,0.0);
 FPartyWorld F;auto G=F.G;G->Score=817;
 G->StartMeme();TestFalse(TEXT("Cannot trigger during gameplay"),G->MemeShowing);
 G->Stage=3;G->StartMeme();TestTrue(TEXT("Results can celebrate"),G->MemeShowing);TestNotNull(TEXT("Current cat mesh actor created"),G->MemeCat);
 const int Count=G->MemeSet.Num();TestTrue(TEXT("Isolated stage exists"),Count>=4);
 G->TickMeme(.9f);TestEqual(TEXT("Celebration does not change money"),G->Score,817);
 TestTrue(TEXT("Mesh really rotates"),IsValid(G->MemeCat)&&!G->MemeCat->GetActorRotation().IsNearlyZero(.01f));
 G->StopMeme();TestFalse(TEXT("Skip immediately shows receipt"),G->MemeShowing);TestEqual(TEXT("Skip keeps results stage"),G->Stage,3);
 for(int I=0;I<10;I++){G->StartMeme();TestEqual(TEXT("Replay replaces stage actors instead of accumulating"),G->MemeSet.Num(),Count);G->TickMeme(G->MemeDuration+.1f);TestFalse(TEXT("Automatically returns to receipt"),G->MemeShowing);}
 G->StartMeme();G->OpenSelection();TestFalse(TEXT("Selection stops meme"),G->MemeShowing);TestEqual(TEXT("Selection clears stage actors"),G->MemeSet.Num(),0);TestNull(TEXT("Selection clears camera"),G->MemeCamera);
 G->Stage=3;G->StartMeme();G->StartRound(false);TestFalse(TEXT("Retry stops meme"),G->MemeShowing);TestEqual(TEXT("Retry clears stage"),G->MemeSet.Num(),0);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabVoiceTest,"BadCatLab.Party.VocalCooldownAndReset",Flags)
bool FLabVoiceTest::RunTest(const FString&){
 FPartyWorld F;auto G=F.G;
 G->Speak(TEXT("Meow"));TestTrue(TEXT("Real meow establishes duration gate"),G->CatVoiceUntil>1);
 const double Until=G->CatVoiceUntil;G->Speak(TEXT("MeowSoft"));TestEqual(TEXT("Soft and strong meows cannot overlap"),G->CatVoiceUntil,Until);
 G->CatVoiceUntil=0;G->SoftMeowUntil=0;G->Speak(TEXT("MeowSoft"));TestTrue(TEXT("Soft cat recording loads"),G->CatVoiceUntil>0);TestTrue(TEXT("Idle meow has ten-second spacing"),G->SoftMeowUntil>=10);
 G->ReceiptGreetingPending=true;G->ResetParty();TestEqual(TEXT("Reset clears cat gate"),G->CatVoiceUntil,0.0);TestEqual(TEXT("Reset clears owner gate"),G->OwnerVoiceUntil,0.0);TestFalse(TEXT("Reset cannot play deferred owner greeting"),G->ReceiptGreetingPending);
 G->Stage=3;G->StartMeme();G->ReceiptGreetingPending=true;G->StopMeme();TestFalse(TEXT("Receipt greeting consumed once"),G->ReceiptGreetingPending);
 const double HomeUntil=G->OwnerVoiceUntil;G->StopMeme();TestEqual(TEXT("Repeated stop cannot repeat greeting"),G->OwnerVoiceUntil,HomeUntil);
 G->ClearMeme();G->ClearVoices();return true;
}
#endif
