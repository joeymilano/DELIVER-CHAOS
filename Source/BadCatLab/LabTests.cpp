#if WITH_DEV_AUTOMATION_TESTS
#include "LabGame.h"
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
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
#endif
