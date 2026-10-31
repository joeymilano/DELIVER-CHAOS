#include "LabResident.h"
#include "LabGame.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimSequence.h"
ALabResident::ALabResident(){PrimaryActorTick.bCanEverTick=true;GetCapsuleComponent()->InitCapsuleSize(28,90);GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);GetCharacterMovement()->bRunPhysicsWithNoController=true;GetCharacterMovement()->bOrientRotationToMovement=true;GetCharacterMovement()->RotationRate=FRotator(0,280,0);GetCharacterMovement()->MaxStepHeight=25;bUseControllerRotationYaw=false;GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;}
void ALabResident::Configure(bool IsGuest){Guest=IsGuest;Home=GetActorLocation();FString Name=Guest?TEXT("Guest"):TEXT("Owner");auto Mesh=LoadObject<USkeletalMesh>(nullptr,*FString::Printf(TEXT("/Game/FabLocal/Characters/%s_Idle"),*Name));GetMesh()->SetSkeletalMesh(Mesh);GetMesh()->SetRelativeLocation(FVector(0,0,-90));GetMesh()->SetRelativeRotation(FRotator(0,-90,0));GetMesh()->SetRelativeScale3D(FVector(.94f));for(auto Clip:{TEXT("Idle"),TEXT("Walk"),TEXT("Run"),TEXT("Confused")})Clips.Add(LoadObject<UAnimSequence>(nullptr,*FString::Printf(TEXT("/Game/FabLocal/Characters/%s_%s_Anim"),*Name,Clip)));}
void ALabResident::ResetResident(){GetCharacterMovement()->SetMovementMode(MOVE_Walking);Anim=INDEX_NONE;Alert=Confused=CatchCooldown=0;Point=0;GetCharacterMovement()->StopMovementImmediately();SetActorLocation(Home,false,nullptr,ETeleportType::TeleportPhysics);}
void ALabResident::Tick(float Dt){Super::Tick(Dt);auto G=Cast<ALabGameMode>(UGameplayStatics::GetGameMode(this));auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(this,0));if(!G)return;SetActorHiddenInGame(!G->Playing());SetActorEnableCollision(G->Playing());if(!G->Playing()){GetCharacterMovement()->StopMovementImmediately();GetCharacterMovement()->DisableMovement();return;}
 if(GetCharacterMovement()->MovementMode==MOVE_None)GetCharacterMovement()->SetMovementMode(MOVE_Walking);
 if(GetActorLocation().Z< -50||FMath::Abs(GetActorLocation().X)>920||FMath::Abs(GetActorLocation().Y)>720)ResetResident();
 if(!Cat)return;
 CatchCooldown=FMath::Max(0.f,CatchCooldown-Dt);Confused=FMath::Max(0.f,Confused-Dt);FVector Delta=Cat->GetActorLocation()-GetActorLocation();float Dist=Delta.Size2D();FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);bool Block=GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation()+FVector(0,0,55),Cat->GetActorLocation(),ECC_Visibility,Q);bool Sees=Dist<650&&(!Block||Hit.GetActor()==Cat)&&FVector::DotProduct(GetActorForwardVector(),Delta.GetSafeNormal2D())>.1f;
 if(Sees&&Cat->Innocent&&Alert>0){Confused=2.5f;Alert=0;G->Quip=TEXT("咦，刚才不是你？");G->QuipUntil=GetWorld()->GetTimeSeconds()+2.5;G->Speak(TEXT("OwnerGood"));}
 else if(Sees&&!Cat->Innocent&&(Cat->Charging||Cat->ActionTime>0||Cat->ZoomTime>0||G->Suspicion>0)){if(Alert<=0)G->WorldSound(TEXT("Alert"),.3f);Alert=4;}
 else Alert=FMath::Max(0.f,Alert-Dt);
 FVector Route[]={FVector(-650,-30,90),FVector(-550,400,90),FVector(550,400,90),FVector(600,-70,90)};FVector Target=Alert>0?Cat->GetActorLocation():Route[(Point+(Guest?2:0))%4];if(FVector::Dist2D(Target,GetActorLocation())<75)Point++;
 FVector Dir=(Target-GetActorLocation()).GetSafeNormal2D();GetCharacterMovement()->MaxWalkSpeed=Alert>0?(Guest?245:265):110;
 if(Confused<=0){FCollisionQueryParams Avoid;Avoid.AddIgnoredActor(this);Avoid.AddIgnoredActor(Cat);FHitResult Obstacle;if(GetWorld()->SweepSingleByChannel(Obstacle,GetActorLocation(),GetActorLocation()+Dir*90,FQuat::Identity,ECC_WorldStatic,FCollisionShape::MakeSphere(30),Avoid)){Dir=FVector::CrossProduct(Obstacle.Normal,FVector::UpVector).GetSafeNormal();if(Guest)Dir=-Dir;}AddMovementInput(Dir);}else GetCharacterMovement()->StopMovementImmediately();
 if(Alert>0&&Dist<85&&FMath::Abs(Delta.Z)<80&&CatchCooldown<=0&&(!Block||Hit.GetActor()==Cat)&&G->CatchCat(Cat)){CatchCooldown=6;Confused=2;Alert=0;}

 int Desired=Confused>0?3:GetVelocity().Size2D()>150?2:GetVelocity().Size2D()>8?1:0;if((Desired!=Anim||!GetMesh()->IsPlaying())&&Clips.IsValidIndex(Desired)&&Clips[Desired]){GetMesh()->PlayAnimation(Clips[Desired],true);Anim=Desired;}if(Desired==1||Desired==2)GetMesh()->SetPlayRate(FMath::Clamp(GetVelocity().Size2D()/(Desired==2?255.f:110.f),.65f,1.4f));
}
