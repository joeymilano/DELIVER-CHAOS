#include "LabGame.h"
#include "LabResident.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

bool ALabProp::CanTakeCatHit() const {
 const auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());
 return Body->IsSimulatingPhysics()||(G&&this==G->PetCamera&&!G->CameraBroken);
}

FVector ALabResident::RearPoint() const{return GetActorLocation()-GetActorForwardVector()*30+FVector(0,0,-5);}

bool ALabResident::SeesCat(const ALabCat* Cat) const {
 if(!IsValid(Cat))return false;
 const FVector Delta=Cat->GetActorLocation()-GetActorLocation();
 if(Delta.Size2D()>650||FVector::DotProduct(GetActorForwardVector(),Delta.GetSafeNormal2D())<=.1f)return false;
 FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
 return !GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation()+FVector(0,0,55),Cat->GetActorLocation(),ECC_Visibility,Q)||Hit.GetActor()==Cat;
}

bool ALabResident::CanBePranked(const ALabCat* Cat) const {
 auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());
 if(!G||!G->Playing()||G->PrankPending||Guest||PrankCooldown>0||Confused>0||!IsValid(Cat)||Cat->CaughtTime>0||Cat->CatchGrace>0)return false;
 const FVector Delta=Cat->GetActorLocation()-GetActorLocation();
 if(Delta.Size2D()>115*FMath::Sqrt(Cat->BodyScale)||FMath::Abs(Cat->GetActorLocation().Z-RearPoint().Z)>100||
    FVector::DotProduct(GetActorForwardVector(),Delta.GetSafeNormal2D())>-.45f||
    FVector::DotProduct(Cat->AttackDirection(),(-Delta).GetSafeNormal2D())<.25f)return false;
 FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);Q.AddIgnoredActor(Cat);
 return !GetWorld()->LineTraceSingleByChannel(Hit,Cat->GetActorLocation()+FVector(0,0,20),RearPoint(),ECC_Visibility,Q);
}

bool ALabResident::TakeRearPrank(ALabCat* Cat){
 if(!CanBePranked(Cat))return false;
 PrankCooldown=14;PrankAnger=5;PrankFlinch=.65f;Confused=.65f;Alert=6;CaptureTime=0;
 GetCharacterMovement()->StopMovementImmediately();GroundPath.Empty();RouteCooldown=0;
 if(auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode())){
  G->BeginRearPrank(this);G->WorldSound(TEXT("Alert"),.35f);G->Speak(TEXT("OwnerCaught"));
 }
 return true;
}

ALabResident* ALabCat::FindRearPrankTarget() const {
 auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());if(!G)return nullptr;
 ALabResident* Best=nullptr;float Distance=MAX_flt;
 for(auto R:G->Residents)if(IsValid(R)&&R->CanBePranked(this)){
  const float D=FVector::DistSquared(GetActorLocation(),R->RearPoint());if(D<Distance){Best=R;Distance=D;}
 }
 return Best;
}

bool ALabCat::RearPawContact(ALabResident* R,FVector& Surface) const {
 if(!IsValid(R)||!R->CanBePranked(this))return false;
 const bool Rig=GetMesh()->GetSkeletalMeshAsset()&&GetMesh()->DoesSocketExist(TEXT("RightFrontFoot"));
 const FVector Shoulder=Rig?GetMesh()->GetSocketLocation(TEXT("RightArm")):GetActorLocation()+AttackDirection()*20;
 const float Limb=Rig?FVector::Dist(Shoulder,GetMesh()->GetSocketLocation(TEXT("RightForeArm")))+
  FVector::Dist(GetMesh()->GetSocketLocation(TEXT("RightForeArm")),GetMesh()->GetSocketLocation(TEXT("RightFrontFoot"))):42*BodyScale;
 // Use the rear hip surface, rather than a single point that a moving paw can
 // pass beside. This region stays behind the resident's centre and rotates
 // with them. IK still uses the real foreleg length without stretching bones.
 const FVector Foot=Rig?GetMesh()->GetSocketLocation(TEXT("RightFrontFoot")):Shoulder+AttackDirection()*Limb;
 const FTransform Frame=R->GetActorTransform();const FVector Centre(-20,0,-5),Radii(12,20,18);
 const FVector Local=Frame.InverseTransformPosition(Foot)-Centre;
 const float Extent=FVector(Local.X/Radii.X,Local.Y/Radii.Y,Local.Z/Radii.Z).Size();
 Surface=Frame.TransformPosition(Centre+Local/FMath::Max(1.f,Extent));
 return FVector::Dist(Shoulder,Surface)<=Limb+4*BodyScale;
}

void ALabGameMode::BeginRearPrank(ALabResident* R){
 if(!Playing()||!IsValid(R)||PrankPending)return;
 PrankPending=true;PrankTarget=R;EscapeProgress=0;
 PrankHitPoint=R->RearPoint();PrankHitUntil=GetWorld()->GetTimeSeconds()+.7;
 Quip=TEXT("拍完就跑！");QuipUntil=GetWorld()->GetTimeSeconds()+2;
}

void ALabGameMode::CancelRearPrank(){PrankPending=false;PrankTarget.Reset();EscapeProgress=0;}

void ALabGameMode::TickRearPrank(float Dt){
 if(!PrankPending)return;
 auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(this,0));auto R=PrankTarget.Get();
 if(!Playing()||!IsValid(Cat)||!IsValid(R)||Cat->CaughtTime>0){CancelRearPrank();return;}
 bool Chased=false;for(auto Other:Residents)if(IsValid(Other)&&Other->Alert>0)Chased=true;
 const bool Escaped=!Chased&&R->PrankAnger<=0&&FVector::Dist2D(Cat->GetActorLocation(),R->GetActorLocation())>=260&&!R->SeesCat(Cat);
 EscapeProgress=Escaped?FMath::Min(EscapeHold,EscapeProgress+Dt):0;
 if(EscapeProgress<EscapeHold)return;
 CancelRearPrank();++EscapedPranks;MischiefBonus+=EscapeBonus;Score+=EscapeBonus;
 QueueMoneyAt(Cat->GetActorLocation()+FVector(0,0,75),EscapeBonus,7000+EscapedPranks);
 Quip=TEXT("溜了溜了 · +¥500");QuipUntil=GetWorld()->GetTimeSeconds()+3;
 UE_LOG(LogTemp,Display,TEXT("REAR_PRANK_ESCAPE bonus=%d total=%d"),EscapeBonus,EscapedPranks);
}
