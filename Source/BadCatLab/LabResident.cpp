#include "LabResident.h"
#include "LabGame.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
ALabResident::ALabResident(){PrimaryActorTick.bCanEverTick=true;GetCapsuleComponent()->InitCapsuleSize(28,90);GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);GetCharacterMovement()->bRunPhysicsWithNoController=true;GetCharacterMovement()->bOrientRotationToMovement=true;GetCharacterMovement()->RotationRate=FRotator(0,280,0);GetCharacterMovement()->MaxStepHeight=3;GetCharacterMovement()->bCanWalkOffLedges=false;GetCharacterMovement()->bEnablePhysicsInteraction=false;GetCharacterMovement()->StandingDownwardForceScale=0;GetCharacterMovement()->InitialPushForceFactor=0;GetCharacterMovement()->PushForceFactor=0;GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_PhysicsBody,ECR_Ignore);GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);bUseControllerRotationYaw=false;GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;}
void ALabResident::Configure(bool IsGuest){
 Guest=IsGuest;Home=GetActorLocation();BuildGroundGrid();ResetResident();Clips.Empty();
 const FString Name=Guest?TEXT("Guest"):TEXT("Owner");FString Folder=TEXT("/Game/FabLocal/Characters");
 USkeletalMesh* Mesh=nullptr;TArray<UAnimSequence*> AvatarClips;
 if(!Guest){
  Mesh=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/ResidentsLocal/Owner_Idle"),nullptr,LOAD_Quiet|LOAD_NoWarn);
  if(Mesh)for(auto Clip:{TEXT("Idle"),TEXT("Walk"),TEXT("Run"),TEXT("Confused")})AvatarClips.Add(LoadObject<UAnimSequence>(nullptr,*FString::Printf(TEXT("/Game/ResidentsLocal/Owner_%s_Anim"),Clip),nullptr,LOAD_Quiet|LOAD_NoWarn));
  // Incomplete local imports must not replace an animated resident with a frozen mesh.
  if(AvatarClips.Num()==4&&!AvatarClips.Contains(nullptr)){Folder=TEXT("/Game/ResidentsLocal");Clips=AvatarClips;}else Mesh=nullptr;
 }
 const bool Avatar=Mesh!=nullptr;
 if(!Mesh)Mesh=LoadObject<USkeletalMesh>(nullptr,*FString::Printf(TEXT("%s/%s_Idle"),*Folder,*Name));
 GetMesh()->SetSkeletalMesh(Mesh);GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
 float Scale=.94f;float Bottom=0;
 if(Avatar){const auto Bounds=Mesh->GetImportedBounds().GetBox();Scale=176.f/FMath::Max(1.f,Bounds.GetSize().Z);Bottom=Bounds.Min.Z*Scale;}
 GetMesh()->SetRelativeScale3D(FVector(Scale));GetMesh()->SetRelativeLocation(FVector(0,0,-90-Bottom));
 if(Clips.IsEmpty())for(auto Clip:{TEXT("Idle"),TEXT("Walk"),TEXT("Run"),TEXT("Confused")})Clips.Add(LoadObject<UAnimSequence>(nullptr,*FString::Printf(TEXT("%s/%s_%s_Anim"),*Folder,*Name,Clip)));
}
void ALabResident::ResetResident(){
 GetCharacterMovement()->SetMovementMode(MOVE_Walking);Anim=INDEX_NONE;Alert=Confused=CatchCooldown=CaptureTime=0;Point=0;
 GroundPath.Empty();PathPoint=0;RouteCooldown=ProgressClock=BlockedPause=0;
 GetCharacterMovement()->StopMovementImmediately();ConsumeMovementInputVector();
 const int Cell=NearestGroundCell(Home);const FVector Safe=Cell==INDEX_NONE?FVector(Home.X,Home.Y,FloorCentre):GroundCell(Cell);
 SetActorLocation(Safe,false,nullptr,ETeleportType::TeleportPhysics);LastSafeFloor=ProgressOrigin=Safe;
}
bool ALabResident::AdvanceCapture(float Dt,bool Eligible){CaptureTime=Eligible?FMath::Min(.5f,CaptureTime+Dt):0;return CaptureTime>=.5f;}
void ALabResident::Tick(float Dt){Super::Tick(Dt);auto G=Cast<ALabGameMode>(UGameplayStatics::GetGameMode(this));auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(this,0));if(!G)return;SetActorHiddenInGame(!G->Playing());SetActorEnableCollision(G->Playing());if(!G->Playing()){CaptureTime=0;GetCharacterMovement()->StopMovementImmediately();GetCharacterMovement()->DisableMovement();return;}
 if(GetCharacterMovement()->MovementMode==MOVE_None)GetCharacterMovement()->SetMovementMode(MOVE_Walking);
 if(GetActorLocation().Z< -30||GetActorLocation().Z>FloorCentre+25||FMath::Abs(GetActorLocation().X)>870||FMath::Abs(GetActorLocation().Y)>670){GetCharacterMovement()->StopMovementImmediately();SetActorLocation(LastSafeFloor,false,nullptr,ETeleportType::TeleportPhysics);GroundPath.Empty();RouteCooldown=.4f;}
 if(FMath::Abs(GetActorLocation().Z-FloorCentre)<8&&CanStandAt(GetActorLocation()))LastSafeFloor=FVector(GetActorLocation().X,GetActorLocation().Y,FloorCentre);
 if(!Cat)return;
 CatchCooldown=FMath::Max(0.f,CatchCooldown-Dt);Confused=FMath::Max(0.f,Confused-Dt);FVector Delta=Cat->GetActorLocation()-GetActorLocation();float Dist=Delta.Size2D();FHitResult Hit;FCollisionQueryParams Q;Q.AddIgnoredActor(this);bool Block=GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation()+FVector(0,0,55),Cat->GetActorLocation(),ECC_Visibility,Q);bool Sees=Dist<650&&(!Block||Hit.GetActor()==Cat)&&FVector::DotProduct(GetActorForwardVector(),Delta.GetSafeNormal2D())>.1f;
 if(Sees&&Cat->Innocent&&Alert>0){Confused=2.5f;Alert=0;G->Quip=TEXT("咦，刚才不是你？");G->QuipUntil=GetWorld()->GetTimeSeconds()+2.5;G->Speak(TEXT("OwnerGood"));}
 else if(Sees&&!Cat->Innocent&&(Cat->Charging||Cat->ActionTime>0||Cat->ZoomTime>0||(G->CameraFlash>0&&G->CameraInView))){if(Alert<=0)G->WorldSound(TEXT("Alert"),.3f);Alert=4;}
 else Alert=FMath::Max(0.f,Alert-Dt);
 const FVector Route[]={FVector(-500,-40,92),FVector(-430,300,92),FVector(430,430,92),FVector(460,-80,92)};
 const FVector Target=Alert>0?Cat->GetActorLocation():Route[(Point+(Guest?2:0))%4];
 GetCharacterMovement()->MaxWalkSpeed=Alert>0?(Guest?245:265):110;
 if(Confused<=0){NavigateFloor(Target,Dt);if(Alert<=0&&GroundPath.IsEmpty()&&FVector::Dist2D(GetActorLocation(),FloorGoal)<50){Point++;RouteCooldown=0;}}
 else {GetCharacterMovement()->StopMovementImmediately();ConsumeMovementInputVector();GroundPath.Empty();RouteCooldown=0;}

 // Contact must be sustained. Breaking reach, sight or alert cancels the grab.
 if(AdvanceCapture(Dt,Alert>0&&Confused<=0&&!Cat->Innocent&&Cat->CatchGrace<=0&&Dist<85&&FMath::Abs(Delta.Z)<80&&CatchCooldown<=0&&(!Block||Hit.GetActor()==Cat))&&G->CatchCat(Cat)){CaptureTime=0;CatchCooldown=8;Confused=2;Alert=0;}

 int Desired=Confused>0?3:GetVelocity().Size2D()>150?2:GetVelocity().Size2D()>8?1:0;if((Desired!=Anim||!GetMesh()->IsPlaying())&&Clips.IsValidIndex(Desired)&&Clips[Desired]){GetMesh()->PlayAnimation(Clips[Desired],true);Anim=Desired;}if(Desired==1||Desired==2)GetMesh()->SetPlayRate(FMath::Clamp(GetVelocity().Size2D()/(Desired==2?255.f:110.f),.65f,1.4f));else GetMesh()->SetPlayRate(1);
}
