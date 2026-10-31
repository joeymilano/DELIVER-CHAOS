#include "LabGame.h"
#include "LabCatalog.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

void ALabCat::CancelContact(){
 AttackPending=false;AttackTarget.Reset();PushTarget.Reset();LastPushedProp.Reset();
 PendingChain=0;ActionElapsed=ActionDuration=ActionTime=0;PawReachAlpha=ReactionTime=0;
 PushCycle=INDEX_NONE;PushContactApplied=false;
}

bool ALabCat::PawContactPoint(ALabProp* Prop,FVector& Surface,float Slack) const {
 auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());
 if(!IsValid(Prop)||Prop->IsHidden()||!Prop->Body->IsSimulatingPhysics()||
    (Prop->Heavy&&!(Traits&CatTrait::Giant))||(G&&Prop==G->PetCamera&&G->CameraBroken))return false;
 const bool Rig=GetMesh()->GetSkeletalMeshAsset()&&GetMesh()->DoesSocketExist(TEXT("RightFrontFoot"));
 const FVector Shoulder=Rig?GetMesh()->GetSocketLocation(TEXT("RightArm")):GetActorLocation()+AttackDirection()*20;
 const float Limb=Rig?FVector::Dist(Shoulder,GetMesh()->GetSocketLocation(TEXT("RightForeArm")))+
  FVector::Dist(GetMesh()->GetSocketLocation(TEXT("RightForeArm")),GetMesh()->GetSocketLocation(TEXT("RightFrontFoot"))):42*BodyScale;
 if(Prop->Body->GetClosestPointOnCollision(Shoulder,Surface)<0)return false;
 if(FVector::Dist(Shoulder,Surface)>Limb+Slack||FVector::DotProduct((Surface-GetActorLocation()).GetSafeNormal2D(),AttackDirection())<.05f)return false;
 FHitResult Hit;FCollisionQueryParams Query;Query.AddIgnoredActor(this);
 if(GetWorld()->LineTraceSingleByChannel(Hit,Shoulder,Surface,ECC_Visibility,Query)&&Hit.GetActor()!=Prop)return false;
 return true;
}

ALabProp* ALabCat::FindPushTarget() const {
 auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());if(!G)return nullptr;
 ALabProp* Best=nullptr;float Distance=MAX_flt;
 for(auto P:G->Props){FVector Surface;if(PawContactPoint(P,Surface,2*BodyScale)){
  const float D=FVector::DistSquared(Surface,GetActorLocation());if(D<Distance){Distance=D;Best=P;}
 }}return Best;
}

void ALabCat::TickContact(float Dt){
 auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());
 if(!G||!G->Playing()||CaughtTime>0){CancelContact();Pushing=false;return;}
 if(UGameplayStatics::IsGamePaused(GetWorld()))return;
 ReactionTime=FMath::Max(0.f,ReactionTime-Dt);PawReachAlpha=0;
 if(ActionTime>0){
  ActionElapsed+=Dt;ActionTime=FMath::Max(0.f,ActionDuration-ActionElapsed);
  if(auto Target=AttackTarget.Get()){
   FVector Surface;if(PawContactPoint(Target,Surface,4*BodyScale)){
    PawTargetWorld=Surface;
    PawReachAlpha=FMath::SmoothStep(.02f,ContactAt,ActionElapsed)*(1-FMath::SmoothStep(ContactAt+.035f,ActionDuration,ActionElapsed));
   }
  }
  if(AttackPending&&ActionElapsed>=ContactAt){
   AttackPending=false;
   // Resolve the original target again at contact, never redirect a committed paw to another prop.
   auto Target=AttackTarget.Get();
   if(IsValid(Target)&&!Target->IsHidden()&&Target->Body->IsSimulatingPhysics()&&
      (!Target->Heavy||(Traits&CatTrait::Giant))){
    const FVector Origin=GetActorLocation()+FVector(0,0,18),Delta=Target->GetActorLocation()-Origin;
    FHitResult Hit;FCollisionQueryParams Query;Query.AddIgnoredActor(this);
    const bool Blocked=GetWorld()->LineTraceSingleByChannel(Hit,Origin,Target->GetActorLocation(),ECC_Visibility,Query)&&Hit.GetActor()!=Target;
    FVector Surface;
    const bool PawCanReach=ActionPounce||!GetMesh()->GetSkeletalMeshAsset()||PawContactPoint(Target,Surface,4*BodyScale);
    if(PawCanReach&&!Blocked&&Delta.Size()<(ActionPounce?245.f:180.f)*FMath::Sqrt(BodyScale)&&FVector::DotProduct(Delta.GetSafeNormal2D(),AttackDirection())>-.2f){
     Target->Strike(PendingChain,PendingImpulse);Impact(Target->Heavy);
    }
   }
  }
 }
 if(Pushing&&!GetCharacterMovement()->IsFalling()&&GetVelocity().Size2D()<55){
  const float Before=PushClock;PushClock+=Dt;PushPhase=FMath::Fmod(PushClock,PushDuration);
  const int32 Cycle=FMath::FloorToInt(PushClock/PushDuration);
  if(Cycle!=PushCycle){PushCycle=Cycle;PushContactApplied=false;PushTarget=FindPushTarget();}
  auto Target=PushTarget.Get();FVector Surface;
  if(PawContactPoint(Target,Surface,2*BodyScale)){
   const FVector Direction=(Target->GetActorLocation()-GetActorLocation()).GetSafeNormal2D();
   AimDirection=Direction;SetActorRotation(FMath::RInterpTo(GetActorRotation(),Direction.Rotation(),Dt,12));
   PawTargetWorld=Surface;
   PawReachAlpha=FMath::SmoothStep(.06f,PushContactTime,PushPhase)*(1-FMath::SmoothStep(PushContactEnd,PushDuration-.12f,PushPhase));
   // One physical push per paw cycle. A short tap before the contact frame does nothing.
   const bool CrossedContact=Before<float(Cycle)*PushDuration+PushContactTime&&PushClock>=float(Cycle)*PushDuration+PushContactTime;
   if(!PushContactApplied&&CrossedContact){
    PushContactApplied=true;
    const int32 Chain=G->Active(Target->ChainId)?Target->ChainId:G->NewChain(false);
    Target->ChainId=Chain;G->Chains[Chain].Last=GetWorld()->GetTimeSeconds();
    const float Speed=FVector::DotProduct(Target->Body->GetPhysicsLinearVelocity(),Direction);
    const float DeltaSpeed=FMath::Clamp(48.f-Speed,0.f,48.f);
    Target->Body->WakeAllRigidBodies();
    Target->Body->AddImpulseAtLocation(Direction*DeltaSpeed*Target->Body->GetMass(),Surface);
    Target->GentlyPushed=true;LastPushedProp=Target;Idle=0;
   }
  }else {PushTarget.Reset();}
 }
 if(auto Prop=LastPushedProp.Get();Prop&&Prop->GetActorLocation().Z<Prop->InitialPosition.Z-10&&Prop->Body->GetPhysicsLinearVelocity().Z< -15){
  // Let the paw withdraw, then settle into innocence. Never take movement control from the player.
  Pushing=false;PushTarget.Reset();LastPushedProp.Reset();PawReachAlpha=0;ReactionTime=.30f;Idle=.5f;
 }
}

void ALabCat::TickCatAnimation(float Dt){
 auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());if(!G)return;
 if(Charging)Charge=FMath::Min(1.25f,Charge+Dt);
 const float Speed=GetVelocity().Size2D();
 if(Speed>12||Charging||Pushing||ZoomTime>0||ActionTime>0||GetCharacterMovement()->IsFalling()){
  Idle=0;ReactionTime=0;
 }else Idle+=Dt;
 const bool WasInnocent=Innocent;
 Innocent=G->Playing()&&Idle>.85f&&ReactionTime<=0&&Speed<12&&!Charging&&!Pushing&&ActionTime<=0&&!GetCharacterMovement()->IsFalling();
 if(Innocent&&!WasInnocent&&(G->Swipes>0||PushContactApplied))G->Speak(TEXT("MeowSoft"));
 const int32 Desired=Charging?4:ActionTime>0?(ActionPounce?7:5):Pushing&&Speed<55?6:Innocent?3:Speed>180?1:Speed>15?2:0;
 if(Desired!=ActiveAnimation){ActiveAnimation=Desired;AnimationTime=0;}
 if(Locomotion.IsValidIndex(Desired)&&Locomotion[Desired]){
  const float Length=Locomotion[Desired]->GetPlayLength();
  if(Desired==5||Desired==7)AnimationTime=ActionElapsed*Length/FMath::Max(.01f,ActionDuration);
  else if(Desired==6)AnimationTime=PushPhase*Length/PushDuration;
  else {const float Rate=Desired==1||Desired==2?FMath::Clamp(Speed/(Desired==1?300.f:100.f),.65f,1.55f):1.f;AnimationTime=FMath::Fmod(AnimationTime+Dt*Rate,FMath::Max(.01f,Length));}
 }
 const float T=GetWorld()->GetTimeSeconds();
 const float Wiggle=Charging?FMath::Sin(T*(15+Charge*12))*FMath::Lerp(2.f,6.f,Charge/1.25f):0;
 const float Lean=FMath::Clamp(FVector::DotProduct(GetVelocity(),GetActorRightVector())*.025f,-7.f,7.f);
 GetMesh()->SetRelativeRotation(FMath::RInterpTo(GetMesh()->GetRelativeRotation(),FRotator(GetCharacterMovement()->IsFalling()?-8:0,-90+Wiggle,Lean),Dt,12));
 GetMesh()->SetRelativeScale3D(FVector(MeshScale));GetMesh()->SetRelativeLocation(FVector(0,0,MeshFloor));
}
