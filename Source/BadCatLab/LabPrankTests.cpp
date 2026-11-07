#if WITH_DEV_AUTOMATION_TESTS
#include "LabGame.h"
#include "LabResident.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "ProceduralMeshComponent.h"

namespace {
constexpr auto Flags=EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter;
struct FPrankWorld {
 UWorld* W;ALabGameMode* G;ALabCat* Cat;ALabResident* Owner;
 FPrankWorld(){
  W=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(W);
  W->SetGameInstance(NewObject<UGameInstance>(W));W->GetWorldSettings()->DefaultGameMode=ALabGameMode::StaticClass();W->SetGameMode(FURL());
  G=CastChecked<ALabGameMode>(W->GetAuthGameMode());G->NoSave=true;G->Progress=NewObject<ULabProgress>(G);
  auto Floor=W->SpawnActor<AActor>();auto Body=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Body);
  Body->SetBoxExtent(FVector(900,700,20));Body->SetCollisionProfileName(TEXT("BlockAll"));Body->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-20));
  Cat=W->SpawnActor<ALabCat>(FVector(-80,0,35),FRotator::ZeroRotator);
  auto PC=W->SpawnActor<APlayerController>();W->AddController(PC);PC->Possess(Cat);
  Owner=W->SpawnActor<ALabResident>(FVector(0,0,92),FRotator::ZeroRotator);Owner->Home=Owner->GetActorLocation();Owner->BuildGroundGrid();G->Residents.Add(Owner);
 }
 ~FPrankWorld(){GEngine->DestroyWorldContext(W);W->DestroyWorld(false);}
 void Hit(){Cat->Press();Cat->Release();Cat->SetActorLocation(FVector(-55,0,64));Cat->TickContact(.19f);}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabRearGeometryTest,"BadCatLab.Pranks.RearContactAndCancellation",Flags)
bool FLabRearGeometryTest::RunTest(const FString&){
 FPrankWorld F;auto R=F.Owner;auto Cat=F.Cat;
 TestEqual(TEXT("Keyboard acquires nearby owner from behind"),Cat->FindRearPrankTarget(),R);
 Cat->SetActorLocation(FVector(70,0,35));Cat->AimDirection=-FVector::ForwardVector;
 TestFalse(TEXT("Front attack is not a rear prank"),R->CanBePranked(Cat));
 Cat->SetActorLocation(FVector(-80,0,35));Cat->AimDirection=FVector::ForwardVector;
 auto Wall=F.W->SpawnActor<AActor>();auto Body=NewObject<UBoxComponent>(Wall);Wall->SetRootComponent(Body);Body->SetBoxExtent(FVector(3,90,100));Body->SetCollisionProfileName(TEXT("BlockAll"));Body->RegisterComponent();Wall->SetActorLocation(FVector(-45,0,100));
 TestFalse(TEXT("Cannot slap through a wall"),R->CanBePranked(Cat));Wall->Destroy();
 Cat->Press();Cat->Release();Cat->TickContact(.17f);TestFalse(TEXT("Windup does not trigger owner"),F.G->PrankPending);
 Cat->CancelContact();Cat->TickContact(.3f);TestFalse(TEXT("Cancelled paw never triggers late"),F.G->PrankPending);
 Cat->Press();Cat->Release();Cat->SetActorLocation(FVector(70,0,64));Cat->TickContact(.19f);
 TestFalse(TEXT("Owner turning/front position before contact invalidates hit"),F.G->PrankPending);
 Cat->SetActorLocation(FVector(-80,0,35));Cat->AimDirection=FVector::ForwardVector;
 F.Hit();TestTrue(TEXT("Actual contact begins escape challenge"),F.G->PrankPending);
 TestEqual(TEXT("No reward before escaping"),F.G->Score,0);TestTrue(TEXT("Owner reacts and starts pursuit"),R->Alert>0&&R->Confused>0);
 TestFalse(TEXT("Repeated taps cannot stack pending prizes"),R->CanBePranked(Cat));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabRearRewardTest,"BadCatLab.Pranks.EscapeRewardAndCaptureFailure",Flags)
bool FLabRearRewardTest::RunTest(const FString&){
 FPrankWorld F;auto G=F.G;auto R=F.Owner;auto Cat=F.Cat;F.Hit();
 G->TickRearPrank(5);TestEqual(TEXT("Standing beside angry owner earns nothing"),G->Score,0);
 Cat->Innocent=true;R->Tick(.1f);TestTrue(TEXT("Immediate innocent sit cannot erase slap pursuit"),R->PrankAnger>0&&R->Alert>0);
 Cat->SetActorLocation(FVector(-500,0,35));R->Alert=R->PrankAnger=0;R->SetActorRotation(FRotator::ZeroRotator);
 G->TickRearPrank(.9f);TestEqual(TEXT("Needs sustained escape"),G->Score,0);
 Cat->SetActorLocation(FVector(400,0,35));G->TickRearPrank(.1f);TestEqual(TEXT("Reappearing in sight cancels escape progress"),G->EscapeProgress,0.f);
 Cat->SetActorLocation(FVector(-500,0,35));G->TickRearPrank(2.1f);
 TestEqual(TEXT("Escaping pays one 500 coin reward"),G->Score,500);TestEqual(TEXT("Receipt separates mischief bonus"),G->MischiefBonus,500);
 TestEqual(TEXT("Successful prank counted once"),G->EscapedPranks,1);TestEqual(TEXT("Coins appear above cat"),G->MoneyBursts.Num(),1);
 G->TickRearPrank(20);TestEqual(TEXT("Cannot repeatedly collect settled prize"),G->Score,500);
 R->ResetResident();Cat->CatchGrace=0;Cat->CaughtTime=0;Cat->SetActorLocation(FVector(-55,0,64));Cat->AimDirection=FVector::ForwardVector;
 TestTrue(TEXT("New attempt after owner reset"),R->TakeRearPrank(Cat));TestTrue(TEXT("Capture still succeeds"),G->CatchCat(Cat));
 TestFalse(TEXT("Capture forfeits pending prize"),G->PrankPending);G->TickRearPrank(20);TestEqual(TEXT("Failed second attempt adds no money"),G->Score,500);
 G->ResetParty();TestEqual(TEXT("Retry clears prank bonuses"),G->MischiefBonus,0);TestEqual(TEXT("Retry clears escape progress"),G->EscapeProgress,0.f);
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLabCameraReachTest,"BadCatLab.Pranks.MountedCameraSwipeAndUnplug",Flags)
bool FLabCameraReachTest::RunTest(const FString&){
 FPrankWorld F;auto G=F.G;auto Cat=F.Cat;G->SpawnPartyProps();auto Camera=G->PetCamera;
 Cat->SetActorLocation(FVector(Camera->GetActorLocation().X-45,Camera->GetActorLocation().Y,35));Cat->AimDirection=FVector::ForwardVector;
 TestFalse(TEXT("Camera remains mounted before attack"),Camera->Body->IsSimulatingPhysics());
 TestTrue(TEXT("Mounted camera can receive cat contact"),Camera->CanTakeCatHit());
 TestEqual(TEXT("Keyboard selects the reachable camera"),Cat->FindTarget(170,.1f),Camera);
 Cat->Press();Cat->Release();Cat->TickContact(.17f);TestFalse(TEXT("Camera needs a real contact frame"),G->CameraBroken);
 Cat->TickContact(.02f);TestTrue(TEXT("One nearby paw knocks camera off"),G->CameraBroken);
 TestEqual(TEXT("Broken camera bills once"),G->Score,399);TestTrue(TEXT("Broken mount releases physics"),Camera->Body->IsSimulatingPhysics());
 TestFalse(TEXT("Broken surveillance overlay disappears"),G->CameraVision->IsVisible());
 Camera->Strike(G->NewChain(false),FVector(20,0,20));TestEqual(TEXT("Repeated camera hits cannot farm money"),G->Score,399);
 G->StartRound(false);Camera=G->PetCamera;
 TestFalse(TEXT("Retry restores live camera"),G->CameraBroken);
 Cat->SetActorLocation(FVector(Camera->GetActorLocation().X-45,Camera->GetActorLocation().Y,35));Cat->AimDirection=FVector::ForwardVector;
 Cat->GetMesh()->TickAnimation(.016f,false);Cat->GetMesh()->RefreshBoneTransforms();
 AddInfo(FString::Printf(TEXT("Mounted E reach: shoulder=%s forearm=%s foot=%s target=%s selected=%s"),*Cat->GetMesh()->GetSocketLocation(TEXT("RightArm")).ToString(),*Cat->GetMesh()->GetSocketLocation(TEXT("RightForeArm")).ToString(),*Cat->GetMesh()->GetSocketLocation(TEXT("RightFrontFoot")).ToString(),*Camera->GetActorLocation().ToString(),*GetNameSafe(Cat->FindPushTarget())));
 Cat->PushStart();Cat->TickContact(.29f);TestFalse(TEXT("Unplugging has brief paw windup"),G->CameraBroken);
 Cat->TickContact(.02f);TestTrue(TEXT("E disconnects mounted camera at contact"),G->CameraBroken);
 TestFalse(TEXT("Unplugging releases the push pose"),Cat->Pushing);TestEqual(TEXT("Unplugging bills camera only once"),G->Score,399);
 return true;
}
#endif
