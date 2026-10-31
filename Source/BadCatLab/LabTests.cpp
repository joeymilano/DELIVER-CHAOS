#if WITH_DEV_AUTOMATION_TESTS
#include "LabGame.h"
#include "LabCatalog.h"
#include "LabResident.h"
#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
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
 FPartyWorld F;auto G=F.G;G->CameraObservation(true,false);TestEqual(TEXT("Caught adds suspicion"),G->Suspicion,1);TestEqual(TEXT("Caught brings owner home sooner"),G->Remaining,177.f);
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
 FPartyWorld F;auto G=F.G;int InitialCount=-1;
 for(int Round=0;Round<20;Round++){
  G->RecipeId=Round%2?2:1;G->StartRound(true);if(InitialCount<0)InitialCount=G->Props.Num();TestEqual(TEXT("Each round restores the same prop count"),G->Props.Num(),InitialCount);
  ALabProp* Bowl=nullptr;for(auto P:G->Props)if(P->PropId==19)Bowl=P;
  TestNotNull(TEXT("Candy bowl exists"),Bowl);if(!Bowl)return false;int C=G->NewChain(true);G->Affect(Bowl,C);TestEqual(TEXT("Bowl releases exactly 24 candies"),G->Props.Num(),InitialCount+24);G->Affect(Bowl,G->NewChain(false));TestEqual(TEXT("Bowl cannot duplicate candies"),G->Props.Num(),InitialCount+24);
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabMemeTest,"BadCatLab.Party.BriefMeowAndOptionalGif",Flags)
bool FLabMemeTest::RunTest(const FString&){
 TestTrue(TEXT("Starts facing camera"),ALabGameMode::MemePose(0).IsNearlyZero());
 TestTrue(TEXT("Meow never spins the model"),ALabGameMode::MemePose(.9f).IsNearlyZero());
 TestTrue(TEXT("Head movement does not rotate whole cat"),ALabGameMode::MemePose(1.45f).IsNearlyZero(.01f));
 TestTrue(TEXT("Finishes facing forward"),ALabGameMode::MemePose(4.8f).IsNearlyZero(.01f));
 TestEqual(TEXT("No camera-like roll"),ALabGameMode::MemePose(.9f).Roll,0.0);
 FPartyWorld F;auto G=F.G;G->Score=817;
 G->StartMeme();TestFalse(TEXT("Cannot trigger during gameplay"),G->MemeShowing);
 G->Stage=3;G->StartMeme();TestTrue(TEXT("Results can celebrate"),G->MemeShowing);TestNotNull(TEXT("Current cat mesh actor created"),G->MemeCat);
 const int Count=G->MemeSet.Num();TestTrue(TEXT("Isolated stage exists"),Count>=4);
 G->TickMeme(.9f);TestEqual(TEXT("Celebration does not change money"),G->Score,817);
 TestTrue(TEXT("Meow keeps the actor facing forward"),IsValid(G->MemeCat)&&G->MemeCat->GetActorRotation().IsNearlyZero(.01f));TestTrue(TEXT("Meow audio triggers once"),G->MeowPlayed);
 G->StopMeme();TestFalse(TEXT("Skip immediately shows receipt"),G->MemeShowing);TestEqual(TEXT("Skip keeps results stage"),G->Stage,3);
 for(int I=0;I<10;I++){G->StartMeme();TestEqual(TEXT("Replay replaces stage actors instead of accumulating"),G->MemeSet.Num(),Count);G->TickMeme(G->MemeDuration+.1f);TestFalse(TEXT("Automatically returns to receipt"),G->MemeShowing);}
 G->ToggleMemeGif();if(G->MemeGifTexture){TestTrue(TEXT("Optional GIF opens"),G->MemeGifShowing);G->TickMeme(5.4f);TestFalse(TEXT("GIF ends after two loops"),G->MemeGifShowing);}
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabWorldUpgradeTest,"BadCatLab.World.FabResidentsAndRoute",Flags)
bool FLabWorldUpgradeTest::RunTest(const FString&){
 if(!LoadObject<UStaticMesh>(nullptr,TEXT("/Game/FabLocal/Furniture/SM_Sofa_41"),nullptr,LOAD_Quiet|LOAD_NoWarn)){AddWarning(TEXT("Acquire local Fab assets to run integration test"));return true;}
 FPartyWorld F;F.G->BuildRoom();TestEqual(TEXT("Two visible opponents exist"),F.G->Residents.Num(),2);
 for(auto R:F.G->Residents){TestTrue(TEXT("Opponent never pushes camera into cat"),R->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Camera)==ECR_Ignore);TestNotNull(TEXT("Resident skeletal model"),R->GetMesh()->GetSkeletalMeshAsset());for(auto A:R->Clips)TestNotNull(TEXT("Resident animation imported"),A);R->Alert=4;R->CatchCooldown=6;R->ResetResident();TestEqual(TEXT("Retry clears pursuit"),R->Alert,0.f);TestEqual(TEXT("Retry clears catch cooldown"),R->CatchCooldown,0.f);}
 FHitResult Hit;bool Floor=F.W->LineTraceSingleByChannel(Hit,FVector(-820,0,100),FVector(-820,0,-50),ECC_Visibility);TestTrue(TEXT("Expanded floor has collision"),Floor);
 bool Catwalk=F.W->LineTraceSingleByChannel(Hit,FVector(400,-555,400),FVector(400,-555,200),ECC_Visibility);TestTrue(TEXT("Elevated route supports the cat"),Catwalk);if(Catwalk)TestTrue(TEXT("Catwalk at intended height"),Hit.Location.Z>260);
 F.G->StartRound(false);TestEqual(TEXT("Challenge lasts three minutes"),F.G->Remaining,180.f);TestTrue(TEXT("New upper and kitchen targets spawned"),F.G->Props.Num()>=37);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabTitleTest,"BadCatLab.UI.TitleNavigation",Flags)
bool FLabTitleTest::RunTest(const FString&){
 FPartyWorld F;auto G=F.G;
 G->OpenTitle();TestEqual(TEXT("Title is not a running round"),G->Stage,6);TestFalse(TEXT("Timer inactive"),G->Playing());
 const int Count=G->TitleSet.Num();TestTrue(TEXT("Title scenery exists"),Count>0);
 G->Retry();TestEqual(TEXT("Retry cannot skip title"),G->Stage,6);
 G->StartFromTitle(true);TestTrue(TEXT("Free play selected"),G->Practice);TestEqual(TEXT("Select cat first"),G->Stage,4);TestEqual(TEXT("Title actors removed"),G->TitleSet.Num(),0);
 G->OpenTitle();TestEqual(TEXT("Reopening does not duplicate scenery"),G->TitleSet.Num(),Count);
 G->ConfirmSelection();TestFalse(TEXT("Normal play resets free mode"),G->Practice);TestEqual(TEXT("Enter opens collection"),G->Stage,4);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabSafetyTest,"BadCatLab.World.BoundsAndCapture",Flags)
bool FLabSafetyTest::RunTest(const FString&){
 FPartyWorld F;auto G=F.G;G->BuildRoom();auto Cat=F.W->SpawnActor<ALabCat>();Cat->ApplyIdentity(0,false);Cat->ResetCat();
 TArray<FVector> Outside={FVector(920,0,100),FVector(-920,0,100),FVector(0,720,100),FVector(0,-720,100),FVector(0,0,-200),FVector(0,0,900)};
 for(auto P:Outside){Cat->SetActorLocation(P);Cat->UpdateSafety();TestTrue(TEXT("Out of room returns to safe interior"),FMath::Abs(Cat->GetActorLocation().X)<820&&FMath::Abs(Cat->GetActorLocation().Y)<620&&Cat->GetActorLocation().Z>0);}
 FCollisionQueryParams Q;Q.AddIgnoredActor(Cat);FHitResult H;TestTrue(TEXT("Solid floor catches downward trace"),F.W->LineTraceSingleByChannel(H,FVector(820,610,30),FVector(820,610,-100),ECC_WorldStatic,Q));
 G->Remaining=100;TestTrue(TEXT("First capture accepted"),G->CatchCat(Cat));TestEqual(TEXT("Capture costs fifteen seconds"),G->Remaining,85.f);TestEqual(TEXT("Two second timeout"),Cat->CaughtTime,2.f);
 TestFalse(TEXT("Second enemy cannot punish during grace"),G->CatchCat(Cat));TestEqual(TEXT("No double charge"),G->TimesCaught,1);
 Cat->Press();Cat->ZoomStart();Cat->PushStart();TestFalse(TEXT("Timeout blocks attack"),Cat->Charging);TestFalse(TEXT("Timeout blocks push"),Cat->Pushing);TestEqual(TEXT("Timeout blocks dash"),Cat->ZoomTime,0.f);
 Cat->ResetCat();TestEqual(TEXT("Retry clears timeout"),Cat->CaughtTime,0.f);TestEqual(TEXT("Retry clears grace"),Cat->CatchGrace,0.f);
 G->Practice=true;TestTrue(TEXT("Free mode still catches"),G->CatchCat(Cat));TestEqual(TEXT("Free mode has no timer penalty"),G->Remaining,85.f);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabResidentPoseTest,"BadCatLab.World.ResidentBoneMotion",Flags)
bool FLabResidentPoseTest::RunTest(const FString&){
 FPartyWorld F;for(bool Guest:{false,true}){auto R=F.W->SpawnActor<ALabResident>();R->Configure(Guest);
  auto Walk=R->Clips[1];if(!TestNotNull(TEXT("Walk exists"),Walk))continue;
  int Id=Walk->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(TEXT("thigh_L"));TestTrue(TEXT("Thigh bone exists"),Id>=0);if(Id<0)continue;
  FTransform A,B;Walk->GetBoneTransform(A,FSkeletonPoseBoneIndex(Id),FAnimExtractContext(.05),false);Walk->GetBoneTransform(B,FSkeletonPoseBoneIndex(Id),FAnimExtractContext(.3),false);
  TestTrue(TEXT("Walk actually rotates the thigh"),A.GetRotation().AngularDistance(B.GetRotation())>.08);
  int Arm=Walk->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(TEXT("upper_arm_L"));
  R->Clips[0]->GetBoneTransform(A,FSkeletonPoseBoneIndex(Arm),FAnimExtractContext(.1),false);
  TestTrue(TEXT("Idle lowers arm away from T pose"),A.GetRotation().AngularDistance(Walk->GetSkeleton()->GetReferenceSkeleton().GetRefBonePose()[Arm].GetRotation())>.5);
 }
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabCameraSafetyTest,"BadCatLab.World.CloseCameraAndMenuResidents",Flags)
bool FLabCameraSafetyTest::RunTest(const FString&){
 FPartyWorld F;F.G->BuildRoom();auto Cat=F.W->SpawnActor<ALabCat>();auto PC=F.W->SpawnActor<APlayerController>();PC->Possess(Cat);
 Cat->SetActorLocation(FVector(820,600,40));Cat->Camera->SetWorldLocation(Cat->GetActorLocation());FMinimalViewInfo View;Cat->CalcCamera(.016f,View);
 TestTrue(TEXT("Compressed camera finds a distant view"),FVector::Dist(View.Location,Cat->GetActorLocation())>150);TestTrue(TEXT("Recovery view stays inside walls"),FMath::Abs(View.Location.X)<=840&&FMath::Abs(View.Location.Y)<=640);
 auto R=F.G->Residents[0];F.G->Stage=6;R->Tick(.016f);TestEqual(TEXT("Menu disables NPC falling"),R->GetCharacterMovement()->MovementMode,MOVE_None);
 F.G->Stage=0;R->Tick(.016f);TestEqual(TEXT("Play resumes NPC movement"),R->GetCharacterMovement()->MovementMode,MOVE_Walking);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabCozyTest,"BadCatLab.World.CozyPropsAndReset",Flags)
bool FLabCozyTest::RunTest(const FString&){
 if(!LoadObject<UStaticMesh>(nullptr,TEXT("/Game/CozyLocal/Crystal_Ball"),nullptr,LOAD_Quiet|LOAD_NoWarn)){AddWarning(TEXT("Nasha licensed local models required for full integration test"));return true;}
 FPartyWorld F;auto G=F.G;G->StartRound(false);int Count=0;TSet<int> Ids;
 for(auto P:G->Props)if(P->PropId>=400&&P->PropId<=408){Count++;Ids.Add(P->PropId);TestTrue(TEXT("New prop has physics collision"),P->Body->IsSimulatingPhysics());TestTrue(TEXT("Price is readable"),P->BaseScore>0);}
 TestEqual(TEXT("Nine interactive tea party props"),Count,9);TestEqual(TEXT("Unique prop IDs"),Ids.Num(),9);
 auto P=*G->Props.FindByPredicate([](auto A){return A->PropId==400;});G->Affect(P,G->NewChain(false));G->Affect(P,G->NewChain(false));TestEqual(TEXT("Crystal billed once"),G->Score,699);
 G->StartRound(false);Count=0;for(auto A:G->Props)if(A->PropId>=400&&A->PropId<=408)Count++;TestEqual(TEXT("Retry rebuilds without duplication"),Count,9);TestEqual(TEXT("Retry clears bill"),G->Score,0);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabMoneyFeedbackTest,"BadCatLab.Party.MoneyFeedbackAndReset",Flags)
bool FLabMoneyFeedbackTest::RunTest(const FString&){
 FPartyWorld F;auto G=F.G;auto Cup=F.Prop(600,89);int Chain=G->NewChain(true);G->Affect(Cup,Chain);G->Affect(Cup,Chain);
 TestEqual(TEXT("One paid object produces one burst"),G->MoneyBursts.Num(),1);TestEqual(TEXT("Burst matches actual bill"),G->MoneyBursts[0].Amount,89);TestEqual(TEXT("Visual feedback cannot duplicate money"),G->Score,89);
 for(int I=0;I<24;I++){auto Candy=F.Prop(700+I,2);G->Affect(Candy,Chain);}
 TestEqual(TEXT("Dense candy feedback merges"),G->MoneyBursts.Num(),2);TestEqual(TEXT("Merged popup retains every candy's value"),G->MoneyBursts[1].Amount,48);TestEqual(TEXT("Actual bill stays exact"),G->Score,137);
 for(int I=0;I<40;I++)G->Affect(F.Prop(800+I,300),Chain);
 TestTrue(TEXT("Screen burst count is bounded"),G->MoneyBursts.Num()<=12);for(auto& B:G->MoneyBursts){TestTrue(TEXT("Coin particles are bounded per burst"),B.CoinCount()<=8);B.Born-=10;}
 G->TickMoneyFeedback();TestEqual(TEXT("Old effects expire"),G->MoneyBursts.Num(),0);TestTrue(TEXT("Animated counter cannot overrun actual bill"),G->DisplayBill<=G->Score);
 G->OpenSelection();TestEqual(TEXT("Returning to menu clears feedback"),G->MoneyBursts.Num(),0);TestEqual(TEXT("Returning to menu clears animated bill"),G->DisplayBill,0.f);TestEqual(TEXT("Returning to menu clears pulse"),G->BillPulse,0.f);return true;
}
#endif
