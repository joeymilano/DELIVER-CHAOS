#if WITH_DEV_AUTOMATION_TESTS
#include "LabGame.h"
#include "LabCatalog.h"
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabScoreTest,"BadCatLab.Scoring.UniqueAndCausal",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLabScoreTest::RunTest(const FString&){
 UWorld* W=UWorld::CreateWorld(EWorldType::Game,false);
 auto G=W->SpawnActor<ALabGameMode>();
 auto A=W->SpawnActor<ALabProp>();A->PropId=1;
 auto B=W->SpawnActor<ALabProp>();B->PropId=2;
 const int C=G->NewChain(true);G->Affect(A,C);G->Affect(A,C);
 TestEqual(TEXT("Repeated object scores once"),G->Score,100);
 TestEqual(TEXT("Repeated object does not add a chain member"),G->Chains[C].Members.Num(),1);
 G->Affect(B,C);TestEqual(TEXT("Second unique object receives multiplier two"),G->Score,300);
 int Other=G->NewChain(false);G->Affect(A,Other);
 TestEqual(TEXT("New chain cannot farm base score"),G->Score,300);
 TestEqual(TEXT("Independent chains retain membership"),G->Chains[Other].Members.Num(),1);
 G->Chains[C].Last=-10;auto D=W->SpawnActor<ALabProp>();D->PropId=3;G->Affect(D,C);
 TestFalse(TEXT("Expired chain cannot score"),D->Scored);
 G->Stage=1;G->Affect(D,Other);TestFalse(TEXT("Fusion cannot score"),D->Scored);
 G->Stage=2;int Pounce=G->NewChain(true);for(int i=4;i<10;i++){auto P=W->SpawnActor<ALabProp>();P->PropId=i;G->Affect(P,Pounce);}
 TestTrue(TEXT("Five objects in one pounce completes challenge"),G->Challenge);
 TestEqual(TEXT("Multiplier capped at five and challenge paid once"),G->Score,2800);
 W->DestroyWorld(false);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabFusionTest,"BadCatLab.Fusion.SelectionAndRules",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLabFusionTest::RunTest(const FString&){
 for(int A=0;A<3;A++)for(int B=0;B<3;B++){
  const int R=LabCatalog::RecipeFor(A,B);
  TestEqual(TEXT("Fusion is symmetric"),R,LabCatalog::RecipeFor(B,A));
  TestEqual(TEXT("Same cat cannot fuse with itself"),R==INDEX_NONE,A==B);
 }
 TestEqual(TEXT("Invalid selection has no recipe"),LabCatalog::RecipeFor(-1,2),INDEX_NONE);
 UWorld* W=UWorld::CreateWorld(EWorldType::Game,false);
 auto G=W->SpawnActor<ALabGameMode>();
 G->OpenSelection();G->SelectCat(2);G->SelectCat(9);
 TestEqual(TEXT("Out of range input preserves selection"),G->SelectedCat,2);
 G->BeginFusionChoice();G->SelectCat(2);
 TestEqual(TEXT("Cannot select main cat as partner"),G->PartnerCat,0);
 G->SelectCat(1);G->ConfirmSelection();
 TestEqual(TEXT("Confirmation starts fusion"),G->Stage,1);
 TestEqual(TEXT("Black plus orange selects mixed recipe"),G->RecipeId,2);
 TestFalse(TEXT("Rules do not apply during presentation"),G->BounceRule());
 G->Stage=2;
 TestTrue(TEXT("Mixed recipe bounces"),G->BounceRule());
 TestTrue(TEXT("Mixed recipe resonates"),G->ResonanceRule());
 G->RecipeId=1;TestFalse(TEXT("Resonance recipe does not bounce"),G->BounceRule());
 G->RecipeId=0;TestFalse(TEXT("Bounce recipe does not resonate"),G->ResonanceRule());
 G->Score=999;G->NewChain(true);G->OpenSelection();
 TestEqual(TEXT("Returning to selection clears score"),G->Score,0);
 TestEqual(TEXT("Returning to selection clears chains"),G->Chains.Num(),0);
 TestEqual(TEXT("Returning to selection clears recipe"),G->RecipeId,INDEX_NONE);
 W->DestroyWorld(false);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabResonanceTest,"BadCatLab.Fusion.ResonanceBudgetAndReset",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FLabResonanceTest::RunTest(const FString&){
 UWorld* W=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(W);auto G=W->SpawnActor<ALabGameMode>();
 for(int Round=0;Round<20;Round++){
  G->Stage=2;G->RecipeId=Round%2?2:1;
  for(int I=0;I<7;I++){auto P=W->SpawnActor<ALabProp>(FVector(I*15,0,50),FRotator::ZeroRotator);P->PropId=I;P->Heavy=I==1;P->Body->SetSimulatePhysics(true);G->Props.Add(P);}
  auto Source=G->Props[0];const int Chain=G->NewChain(true);G->Affect(Source,Chain);G->TryResonate(Source,Chain,FVector::ForwardVector);
  const int Expected=G->RecipeId==2?3:4;
  TestEqual(TEXT("Pulse obeys recipe target budget"),G->Chains[Chain].Members.Num(),Expected);
  TestFalse(TEXT("Pulse excludes heavy furniture"),G->Props[1]->Scored);
  G->TryResonate(Source,Chain,FVector::ForwardVector);
  TestEqual(TEXT("Pulse can only fire once per chain"),G->Chains[Chain].Members.Num(),Expected);
  G->OpenSelection();TestEqual(TEXT("Reset clears props"),G->Props.Num(),0);TestEqual(TEXT("Reset clears score"),G->Score,0);TestEqual(TEXT("Reset clears chain state"),G->Chains.Num(),0);
 }
 GEngine->DestroyWorldContext(W);W->DestroyWorld(false);return true;
}
#endif
