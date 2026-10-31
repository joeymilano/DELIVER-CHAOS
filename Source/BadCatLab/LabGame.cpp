#include "LabGame.h"
#include "LabResident.h"
#include "LabCatalog.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundBase.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
static FLinearColor Cream(0.90f,0.83f,0.72f), Ink(0.025f,0.028f,0.028f), Orange(1,0.145f,0.025f), Blue(0.43f,0.59f,0.64f);
static UStaticMesh* Shape(const TCHAR* Name){return LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Name,Name));}
static UStaticMesh* ArtShape(const TCHAR* Name){return LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/Art/Props/%s.%s"),Name,Name));}
static void Paint(UStaticMeshComponent* C,FLinearColor Color){
 UMaterialInterface* M=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/M_Palette.M_Palette"));
 if(!M) M=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
 if(M){auto D=UMaterialInstanceDynamic::Create(M,C);D->SetVectorParameterValue(TEXT("Color"),Color);bool Applied=false;for(int32 I=0;I<C->GetNumMaterials();I++){if(C->GetMaterial(I)&&C->GetMaterial(I)->GetName().Contains(TEXT("Prop_Base"))){C->SetMaterial(I,D);Applied=true;}}if(!Applied&&C->GetNumMaterials()<=1)C->SetMaterial(0,D);}
}
static ALabGameMode* GM(UWorld* W){return Cast<ALabGameMode>(W->GetAuthGameMode());}
ALabProp::ALabProp(){PrimaryActorTick.bCanEverTick=true; Body=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));SetRootComponent(Body);Body->SetStaticMesh(Shape(TEXT("Cube")));Body->SetCollisionProfileName(TEXT("PhysicsActor"));Body->SetNotifyRigidBodyCollision(true);Body->SetUseCCD(true);Body->OnComponentHit.AddDynamic(this,&ALabProp::Hit);}
void ALabProp::Configure(int32 Id,FVector Scale,FLinearColor Color,bool Ball,bool IsHeavy,bool Bounce){
 PropId=Id;Heavy=IsHeavy;Mutated=Bounce&&!Heavy;BaseScore=Id==15?4999:Id==13?399:Id==12?299:Id==0||Id==10||Id==17?89:Id==1?59:Id>=2&&Id<=4?39:Id==6||Id==11?25:49;
 ItemName=Id==15?TEXT("电视"):Id==13?TEXT("台灯"):Id==12?TEXT("小车"):Id==0||Id==10||Id==17?TEXT("杯子"):Id==1?TEXT("托盘"):Id>=2&&Id<=4?TEXT("积木"):Id==6||Id==11?TEXT("玩具球"):TEXT("书");InitialPosition=GetActorLocation();
 const TCHAR* Names[]={TEXT("SM_Cup"),TEXT("SM_Tray"),TEXT("SM_RoundedBlock"),TEXT("SM_RoundedBlock"),TEXT("SM_RoundedBlock"),TEXT("SM_Book"),nullptr,TEXT("SM_Book"),TEXT("SM_Book"),TEXT("SM_Book"),TEXT("SM_Cup"),nullptr,TEXT("SM_Car"),TEXT("SM_Lamp"),TEXT("SM_Book"),TEXT("SM_TV"),TEXT("SM_Book"),TEXT("SM_Cup")};auto Mesh=(Id>=0&&Id<18&&Names[Id])?ArtShape(Names[Id]):nullptr;Body->SetStaticMesh(Mesh?Mesh:Shape(Ball?TEXT("Sphere"):TEXT("Cube")));Body->SetWorldScale3D(Scale);Paint(Body,Color);
 Physics=NewObject<UPhysicalMaterial>(this);Physics->Friction=.45f;Physics->Restitution=Mutated?.93f:.18f;Physics->bOverrideRestitutionCombineMode=true;Physics->RestitutionCombineMode=EFrictionCombineMode::Max;Body->SetPhysMaterialOverride(Physics);
 Body->SetSimulatePhysics(true);Body->SetMassOverrideInKg(NAME_None,Heavy?18.f:1.5f);Body->SetLinearDamping(Mutated?.12f:.5f);Body->SetAngularDamping(.7f);
 Body->SetPhysicsMaxAngularVelocityInDegrees(Heavy?100.f:650.f);
 if(Heavy){Body->BodyInstance.bLockXTranslation=true;Body->BodyInstance.bLockYTranslation=true;Body->BodyInstance.bLockZRotation=true;Body->BodyInstance.bLockXRotation=true;Body->BodyInstance.SetDOFLock(EDOFMode::SixDOF);}
}
void ALabProp::Strike(int32 Chain,FVector V){auto G=GM(GetWorld());if(!G||!G->Playing())return;if(Heavy&&(G->CurrentTraits()&CatTrait::Giant)){Body->BodyInstance.bLockXTranslation=false;Body->BodyInstance.bLockYTranslation=false;Body->BodyInstance.SetDOFLock(EDOFMode::SixDOF);}ChainId=Chain;G->Affect(this,Chain);Body->AddImpulse(V,NAME_None,true);G->TryResonate(this,Chain,V.GetSafeNormal());}
void ALabProp::Hit(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,FVector Impulse,const FHitResult& R){
 auto G=GM(GetWorld());if(!G||!G->Playing())return;
 auto P=Cast<ALabProp>(Other);
 if(P && Impulse.Size()>15){if(G->Active(ChainId)&&!G->Active(P->ChainId)){P->ChainId=ChainId;G->Affect(P,ChainId);}else if(!G->Active(ChainId)&&G->Active(P->ChainId)){ChainId=P->ChainId;G->Affect(this,ChainId);}}
 if(Mutated && G->Active(ChainId)&&Impulse.Size()>80 && Bounces<4){Bounces++;Body->AddImpulse(R.ImpactNormal*(160),NAME_None,true);if(Bounces==4){Physics->Restitution=.15f;Body->SetPhysMaterialOverride(Physics);Body->SetLinearDamping(2);}}
}
void ALabProp::Tick(float Dt){Super::Tick(Dt);if(!Scored){auto G=GM(GetWorld());if(G&&G->Playing()&&G->Active(ChainId)&&(GetActorLocation().Z<InitialPosition.Z-10||FVector::Dist2D(GetActorLocation(),InitialPosition)>35))G->Affect(this,ChainId);}FVector V=Body->GetPhysicsLinearVelocity();if(V.Size()>1800)Body->SetPhysicsLinearVelocity(V.GetClampedToMaxSize(1800));if(GetActorLocation().Z< -150 || GetActorLocation().Size()>2500){Body->SetSimulatePhysics(false);SetActorHiddenInGame(true);SetActorEnableCollision(false);}}
ALabCat::ALabCat(){
 PrimaryActorTick.bCanEverTick=true;GetCapsuleComponent()->InitCapsuleSize(20,30);GetCharacterMovement()->MaxWalkSpeed=290;GetCharacterMovement()->JumpZVelocity=390;GetCharacterMovement()->AirControl=.8f;GetCharacterMovement()->bOrientRotationToMovement=true;GetCharacterMovement()->RotationRate=FRotator(0,800,0);bUseControllerRotationYaw=false;
 Arm=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));Arm->SetupAttachment(RootComponent);Arm->TargetArmLength=430;Arm->SocketOffset=FVector::ZeroVector;Arm->bUsePawnControlRotation=true;Arm->bEnableCameraLag=true;Arm->CameraLagSpeed=14;Arm->CameraLagMaxDistance=65;Arm->bUseCameraLagSubstepping=true;Arm->TargetOffset=FVector(0,0,40);Arm->ProbeSize=16;
 HeroMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeroMesh"));HeroMesh->SetupAttachment(RootComponent);HeroMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));Camera->SetupAttachment(Arm);Camera->FieldOfView=72;
 Visual=CreateDefaultSubobject<USceneComponent>(TEXT("Cat"));Visual->SetupAttachment(RootComponent);Visual->SetRelativeLocation(FVector(0,0,-23));
 auto Part=[&](const TCHAR* Name,const TCHAR* Mesh,FVector Pos,FVector Scale){auto C=CreateDefaultSubobject<UStaticMeshComponent>(Name);C->SetupAttachment(Visual);C->SetStaticMesh(Shape(Mesh));C->SetRelativeLocation(Pos);C->SetRelativeScale3D(Scale);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);return C;};
 Part(TEXT("Body"),TEXT("Sphere"),FVector(-5,0,16),FVector(.6,.38,.38));Part(TEXT("Head"),TEXT("Sphere"),FVector(21,0,30),FVector(.44,.46,.44));
 Part(TEXT("Patch"),TEXT("Sphere"),FVector(38,-12,33),FVector(.12,.2,.22));
 Part(TEXT("EyeL"),TEXT("Sphere"),FVector(41,-12,34),FVector(.04,.055,.065));Part(TEXT("EyeR"),TEXT("Sphere"),FVector(41,12,34),FVector(.04,.055,.065));
 Part(TEXT("Nose"),TEXT("Sphere"),FVector(44,0,26),FVector(.045,.065,.035));
 Part(TEXT("EarL"),TEXT("Cone"),FVector(20,-16,52),FVector(.15,.17,.25));Part(TEXT("EarR"),TEXT("Cone"),FVector(20,16,52),FVector(.15,.17,.25));
 for(int i=0;i<4;i++)Paws.Add(Part(*FString::Printf(TEXT("Paw%d"),i),TEXT("Sphere"),FVector(i<2?14:-22,i%2?14:-14,4),FVector(.18,.14,.18)));
 Tail=Part(TEXT("Tail"),TEXT("Cylinder"),FVector(-36,0,28),FVector(.11,.11,.48));Tail->SetRelativeRotation(FRotator(-30,0,0));
}
void ALabCat::BeginPlay(){Super::BeginPlay();TArray<UStaticMeshComponent*> Parts;GetComponents(Parts);for(auto C:Parts){if(C==HeroMesh)continue;FString N=C->GetName();Paint(C,N==TEXT("Patch")||N==TEXT("Tail")||N==TEXT("Nose")||N==TEXT("EyeR")||N.StartsWith(TEXT("Ear"))?Ink:Cream);}if(Controller)Controller->SetControlRotation(FRotator(-32,0,0));}
void ALabCat::SetupPlayerInputComponent(UInputComponent* I){Super::SetupPlayerInputComponent(I);I->BindKey(EKeys::M,IE_Pressed,this,&ALabCat::ToggleMusic).bExecuteWhenPaused=true;I->BindAxis("Forward",this,&ALabCat::Forward);I->BindAxis("Right",this,&ALabCat::Right);I->BindAxis("Turn",this,&ALabCat::Turn);I->BindAxis("Look",this,&ALabCat::Look);I->BindAction("Attack",IE_Pressed,this,&ALabCat::Press);I->BindAction("Attack",IE_Released,this,&ALabCat::Release);I->BindAction("Jump",IE_Pressed,this,&ALabCat::Hop);I->BindAction("Retry",IE_Pressed,this,&ALabCat::Retry).bExecuteWhenPaused=true;I->BindAction("Pause",IE_Pressed,this,&ALabCat::Pause).bExecuteWhenPaused=true;I->BindAction("HUD",IE_Pressed,this,&ALabCat::ToggleHUD);I->BindAction("Camera",IE_Pressed,this,&ALabCat::ToggleCamera);I->BindAction("Tutorial",IE_Pressed,this,&ALabCat::Tutorial).bExecuteWhenPaused=true;I->BindAction("One",IE_Pressed,this,&ALabCat::PickOne);I->BindAction("Two",IE_Pressed,this,&ALabCat::PickTwo);I->BindAction("Three",IE_Pressed,this,&ALabCat::PickThree);I->BindAction("Four",IE_Pressed,this,&ALabCat::PickFour);I->BindAction("Push",IE_Pressed,this,&ALabCat::PushStart);I->BindAction("Push",IE_Released,this,&ALabCat::PushStop);I->BindAction("Zoom",IE_Pressed,this,&ALabCat::ZoomStart);I->BindAction("Zoom",IE_Released,this,&ALabCat::ZoomStop);I->BindAction("Meow",IE_Pressed,this,&ALabCat::Meow);I->BindAction("Shake",IE_Pressed,this,&ALabCat::ToggleShake).bExecuteWhenPaused=true;I->BindAction("Practice",IE_Pressed,this,&ALabCat::TogglePractice);I->BindAction("Confirm",IE_Pressed,this,&ALabCat::ConfirmSelection);I->BindAction("Select",IE_Pressed,this,&ALabCat::OpenSelection).bExecuteWhenPaused=true;}
void ALabCat::Forward(float V){auto G=GM(GetWorld());if(G&&G->Playing()&&CaughtTime<=0){if(ZoomTime<=0)AddMovementInput(FRotator(0,GetControlRotation().Yaw,0).Vector(),V);if(V!=0)Idle=0;}}
void ALabCat::Right(float V){auto G=GM(GetWorld());if(G&&G->Playing()&&CaughtTime<=0){if(ZoomTime<=0)AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V);if(V!=0)Idle=0;}}
void ALabCat::Turn(float V){if(!FixedCamera&&GM(GetWorld())&&GM(GetWorld())->Playing())AddControllerYawInput(FMath::Clamp(V,-3.f,3.f));}
void ALabCat::Look(float V){if(!FixedCamera&&GM(GetWorld())&&GM(GetWorld())->Playing())AddControllerPitchInput(FMath::Clamp(V,-3.f,3.f));}
void ALabCat::TogglePractice(){if(auto G=GM(GetWorld()))G->SetPractice();}
void ALabCat::Press(){auto G=GM(GetWorld());if(G&&G->Playing()&&CaughtTime<=0&&!UGameplayStatics::IsGamePaused(GetWorld())){Charging=true;Innocent=false;Pushing=false;Charge=0;Idle=0;}}
void ALabCat::Release(){
 auto G=GM(GetWorld());if(!Charging||!G||!G->Playing())return;Charging=false;
 const bool Pounce=Charge>=.25f;const bool Super=Pounce&&Charge>=1.05f&&G->Pounces>0;
 const int32 Chain=G->NewChain(Pounce);G->Swipes++;if(Super){G->Pounces--;G->UsedPounces++;}
 FVector Dir=AttackDirection();auto Target=FindTarget(Pounce?235:150,.10f);
 if(Target){Dir=(Target->GetActorLocation()-GetActorLocation()).GetSafeNormal2D();}
 G->WorldSound(TEXT("Swipe"),.3f);SetActorRotation(Dir.Rotation());ActionPounce=Pounce;ActionTime=Pounce?.43f:.36f;ActiveAnimation=INDEX_NONE;Idle=0;
 const float Power=(Super?1500.f:Pounce?FMath::Lerp(620.f,1000.f,FMath::Clamp(Charge,0.f,1.f)):LabCatalog::Cat(G->SelectedCat).Swipe)*(Traits&CatTrait::Giant?1.65f:1.f);
 if(Target){Target->Strike(Chain,Dir*Power+FVector(0,0,Pounce?200:110));Impact(Target->Heavy);}
 if(Pounce)LaunchCharacter(Dir*(Super?1050:FMath::Lerp(480.f,780.f,FMath::Clamp(Charge,0.f,1.f)))+FVector(0,0,Super?260:180),true,true);
}
void ALabCat::Hop(){if(auto G=GM(GetWorld())){if(G->Stage==3){G->ToggleMemeGif();}else if(G->Stage==5){G->RecipeId=INDEX_NONE;G->StartRound(false);}else if(G->Playing()&&CaughtTime<=0){Jump();Innocent=false;Idle=0;}}}
void ALabCat::Retry(){UGameplayStatics::SetGamePaused(GetWorld(),false);if(auto G=GM(GetWorld()))G->Retry();}
void ALabCat::Tutorial(){UGameplayStatics::SetGamePaused(GetWorld(),false);if(auto G=GM(GetWorld()))G->Retry(true);}
void ALabCat::Pause(){auto G=GM(GetWorld());if(!G)return;if(G->Stage==6){return;}if(G->Stage==4||G->Stage==5){G->OpenTitle();return;}Charging=false;Pushing=false;ZoomTime=0;Charge=0;const bool Paused=!UGameplayStatics::IsGamePaused(GetWorld());UGameplayStatics::SetGamePaused(GetWorld(),Paused);G->SetMenuInput(Paused||G->Stage==3);if(IsValid(G->MemeAudio))G->MemeAudio->SetPaused(Paused);G->PauseVoices(Paused);}
void ALabCat::ToggleHUD(){if(auto G=GM(GetWorld()))G->HideHUD=!G->HideHUD;}
void ALabCat::ToggleCamera(){FixedCamera=!FixedCamera;Arm->bUsePawnControlRotation=!FixedCamera;Arm->SetRelativeRotation(FRotator(-40,-30,0));Arm->TargetArmLength=FixedCamera?650:430;}
void ALabCat::ResetCat(){if(auto PC=Cast<APlayerController>(Controller)){PC->PlayerCameraManager->ViewPitchMin=-65;PC->PlayerCameraManager->ViewPitchMax=-10;PC->FlushPressedKeys();}CaughtTime=CatchGrace=0;GetCharacterMovement()->SetMovementMode(MOVE_Walking);SetActorLocation(FVector(-220,0,30*BodyScale+8),false,nullptr,ETeleportType::TeleportPhysics);GetCharacterMovement()->StopMovementImmediately();Charging=false;Pushing=false;ZoomTime=0;ZoomCooldown=0;MeowCooldown=0;AutoMeowCooldown=0;PushClock=0;FallSpeed=0;Charge=0;Idle=0;ActionTime=0;ImpactTime=0;Innocent=false;ActionPounce=false;GetCharacterMovement()->ClearAccumulatedForces();LastSafePosition=GetActorLocation();GetMesh()->SetVisibility(true);HeroMesh->SetVisibility(true);Arm->SocketOffset=FVector::ZeroVector;SetActorRotation(FRotator::ZeroRotator);if(Controller)Controller->SetControlRotation(FRotator(-32,0,0));}
void ALabCat::SetMutation(bool On){auto G=GM(GetWorld());ApplyIdentity(G?G->SelectedCat:0,On);}
void ALabCat::ApplyIdentity(int32 Id,bool Fused){
 Traits=GM(GetWorld())?GM(GetWorld())->CurrentTraits():LabCatalog::Cat(Id).Traits;BodyScale=Traits&CatTrait::Giant?1.8f:1.f;GetCapsuleComponent()->SetCapsuleSize(20*BodyScale,30*BodyScale);
 const auto& D=LabCatalog::Cat(Id);GetCharacterMovement()->MaxWalkSpeed=D.Speed;GetCharacterMovement()->JumpZVelocity=D.Jump;
 const FString MeshPath(D.Mesh);const FString Folder=FPaths::GetPath(MeshPath);
 const FString RigPath=MeshPath.Replace(TEXT("SM_"),TEXT("SK_"));auto Rig=LoadObject<USkeletalMesh>(nullptr,*RigPath);
 GetMesh()->SetSkeletalMesh(Rig);GetMesh()->SetVisibility(Rig!=nullptr);GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);Locomotion.Empty();ActiveAnimation=INDEX_NONE;
 if(Rig){const FBox B=Rig->GetImportedBounds().GetBox();const float Scale=64.f*BodyScale/FMath::Max(1.f,B.GetSize().Z);MeshScale=Scale;MeshFloor=-30*BodyScale-B.Min.Z*Scale;GetMesh()->SetRelativeScale3D(FVector(Scale));GetMesh()->SetRelativeLocation(FVector(0,0,MeshFloor));GetMesh()->SetRelativeRotation(FRotator(0,-90,0));for(const TCHAR* A:{TEXT("A_Idle"),TEXT("A_Run"),TEXT("A_Walk"),TEXT("A_Sit"),TEXT("A_Charge"),TEXT("A_Swipe"),TEXT("A_Push"),TEXT("A_Pounce")})Locomotion.Add(LoadObject<UAnimSequence>(nullptr,*(Folder/FString(A))));}
 UStaticMesh* M=Rig?nullptr:LoadObject<UStaticMesh>(nullptr,D.Mesh);HeroMesh->EmptyOverrideMaterials();HeroMesh->SetStaticMesh(M);HeroMesh->SetVisibility(M!=nullptr);Visual->SetVisibility(M==nullptr&&Rig==nullptr,true);
 if(M){const FBox B=M->GetBoundingBox();const float Scale=64.f/FMath::Max(1.f,B.GetSize().Z);HeroMesh->SetRelativeScale3D(FVector(Scale));HeroMesh->SetRelativeLocation(FVector(0,0,-30-B.Min.Z*Scale));HeroMesh->SetRelativeRotation(FRotator(0,-90,0));}
 TArray<UStaticMeshComponent*> Parts;GetComponents(Parts);for(auto C:Parts){if(C==HeroMesh||FusionParts.Contains(C))continue;FString N=C->GetName();Paint(C,N==TEXT("Nose")||N.StartsWith(TEXT("Eye"))?Ink:N==TEXT("Patch")||N==TEXT("Tail")?Id==1?FLinearColor(.6,.12,.025):Ink:D.Color);}
 auto G=GM(GetWorld());const FLinearColor Accent=G&&G->RecipeId>=0?LabCatalog::Recipe(G->RecipeId).Color:Orange;
 for(auto Part:FusionParts)if(IsValid(Part))Part->DestroyComponent();FusionParts.Empty();
 if((Traits&CatTrait::Giant)&&Id!=3){for(int I=0;I<6;I++){auto P=NewObject<UStaticMeshComponent>(this);P->SetStaticMesh(Shape(TEXT("Cone")));P->SetupAttachment(RootComponent);P->RegisterComponent();P->SetCollisionEnabled(ECollisionEnabled::NoCollision);P->SetRelativeLocation(FVector(10+(I/2)*4,I%2?13:-13,14-(I/2)*8)*BodyScale);P->SetRelativeRotation(FRotator(180,0,I%2?-25:25));P->SetRelativeScale3D(FVector(.16,.13,.25)*BodyScale);Paint(P,FLinearColor(.44,.25,.11));FusionParts.Add(P);}}
 if(Fused&&G&&G->RecipeId>=0){const auto& R=LabCatalog::Recipe(G->RecipeId);auto AddAccessory=[&](const TCHAR* Name,FVector Pos,FVector Scale){auto M=ArtShape(Name);if(!M)return;auto P=NewObject<UStaticMeshComponent>(this);P->SetStaticMesh(M);P->SetupAttachment(RootComponent);P->RegisterComponent();P->SetCollisionEnabled(ECollisionEnabled::NoCollision);P->SetRelativeLocation(Pos);P->SetRelativeScale3D(Scale);FusionParts.Add(P);};if(Traits&CatTrait::Wild)AddAccessory(TEXT("SM_Spring"),FVector(-20,0,22),FVector(.13,.13,.30));if(Traits&CatTrait::Loud){AddAccessory(TEXT("SM_Speaker"),FVector(-5,-17,8),FVector(.13,.10,.20));AddAccessory(TEXT("SM_Speaker"),FVector(-5,17,8),FVector(.13,.10,.20));}for(auto P:Paws)Paint(P,Accent);}
}
void ALabCat::PickOne(){if(auto G=GM(GetWorld()))G->SelectCat(0);}
void ALabCat::PickTwo(){if(auto G=GM(GetWorld()))G->SelectCat(1);}
void ALabCat::PickThree(){if(auto G=GM(GetWorld()))G->SelectCat(2);}
void ALabCat::PickFour(){if(auto G=GM(GetWorld()))G->SelectCat(3);}
void ALabCat::ConfirmSelection(){if(auto G=GM(GetWorld()))G->ConfirmSelection();}
void ALabCat::OpenSelection(){UGameplayStatics::SetGamePaused(GetWorld(),false);if(auto G=GM(GetWorld()))G->OpenSelection();}
void ALabCat::Tick(float Dt){
 Super::Tick(Dt);auto G=GM(GetWorld());if(!G)return;
 CatchGrace=FMath::Max(0.f,CatchGrace-Dt);CaughtTime=FMath::Max(0.f,CaughtTime-Dt);
 if(CaughtTime>0){GetCharacterMovement()->StopMovementImmediately();ConsumeMovementInputVector();}
 UpdateSafety();TickInstincts(Dt);Idle+=Dt;ActionTime=FMath::Max(0.f,ActionTime-Dt);
 const float Speed=GetVelocity().Size2D(),T=GetWorld()->GetTimeSeconds();
 if(Speed>12||Charging||Pushing||ZoomTime>0)Idle=0;
 const bool WasInnocent=Innocent;
 Innocent=G->Playing()&&Idle>.85f&&Speed<12&&!GetCharacterMovement()->IsFalling();
 if(Innocent&&!WasInnocent&&G->Swipes>0)G->Speak(TEXT("MeowSoft"));
 if(Charging){Charge=FMath::Min(1.25f,Charge+Dt);DrawDebugDirectionalArrow(GetWorld(),GetActorLocation(),GetActorLocation()+AttackDirection()*(90+Charge*110),20,Charge>=1.05f&&G->Pounces>0?FColor::Yellow:FColor(255,106,43),false,0,0,3);}
 if(GetMesh()->GetSkeletalMeshAsset()){
  const int Desired=Charging?4:Pushing?6:ActionTime>0?(ActionPounce?7:5):Innocent?3:Speed>180?1:Speed>15?2:0;
  if(Desired!=ActiveAnimation&&Locomotion.IsValidIndex(Desired)&&Locomotion[Desired]){GetMesh()->PlayAnimation(Locomotion[Desired],Desired!=5&&Desired!=7);ActiveAnimation=Desired;}
  const float Wiggle=Charging?FMath::Sin(T*(15+Charge*12))*FMath::Lerp(2.f,6.f,Charge/1.25f):0;
  const float Lean=FMath::Clamp(FVector::DotProduct(GetVelocity(),GetActorRightVector())*.025f,-7.f,7.f);
  GetMesh()->SetRelativeRotation(FMath::RInterpTo(GetMesh()->GetRelativeRotation(),FRotator(GetCharacterMovement()->IsFalling()?-8:0,-90+Wiggle,Lean),Dt,12));
  if(ActiveAnimation==1||ActiveAnimation==2)GetMesh()->SetPlayRate(FMath::Clamp(Speed/(ActiveAnimation==1?300.f:100.f),.65f,1.55f));else GetMesh()->SetPlayRate(1.f);
  GetMesh()->SetRelativeScale3D(FVector(MeshScale));
  GetMesh()->SetRelativeLocation(FVector(0,0,MeshFloor));
 }
 if(G->Playing()&&Speed>35&&!GetCharacterMovement()->IsFalling()){StepClock+=Dt;if(StepClock>(Speed>180?.22f:.36f)){StepClock=0;G->WorldSound(TEXT("Footstep"),.12f);}}
 Visual->SetRelativeRotation(FRotator(ActionTime>0?-18:Innocent?12:0,0,0));
 if(Shake&&ImpactTime>0){ImpactTime=FMath::Max(0.f,ImpactTime-Dt);Arm->SocketOffset=FVector(0,FMath::Sin(T*110)*ImpactTime*32,FMath::Sin(T*87)*ImpactTime*20);}else Arm->SocketOffset=FVector::ZeroVector;


}
ALabGameMode::ALabGameMode(){PrimaryActorTick.bCanEverTick=true;DefaultPawnClass=ALabCat::StaticClass();HUDClass=ALabHUD::StaticClass();}
void ALabGameMode::BeginPlay(){Super::BeginPlay();LoadProgress();BuildRoom();OpenTitle();
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("CatArtReview"))){int32 ReviewCat=0,ReviewRecipe=-1;FParse::Value(FCommandLine::Get(),TEXT("ReviewCat="),ReviewCat);FParse::Value(FCommandLine::Get(),TEXT("ReviewRecipe="),ReviewRecipe);SelectedCat=FMath::Clamp(ReviewCat,0,3);RecipeId=FMath::Clamp(ReviewRecipe,-1,5);StartRound(RecipeId>=0);if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->FixedCamera=true;Cat->Arm->bUsePawnControlRotation=false;Cat->Arm->SetRelativeRotation(FRotator(-25,-25,0));Cat->Arm->TargetArmLength=260;Cat->SetActorRotation(FRotator(0,155,0));}}
 // Deterministic screen review is opt-in, Development-only and never touches player saves.
 FString Review;
 if(NoSave&&FParse::Value(FCommandLine::Get(),TEXT("UIReview="),Review)){
  if(Review!=TEXT("title"))OpenSelection();
  if(Review!=TEXT("locked"))Progress->UnlockedMask=15;
  if(Review==TEXT("cozy")){
   StartRound(false);Practice=true;
   if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->SetActorLocation(FVector(-260,290,40));Cat->FixedCamera=true;Cat->Arm->bUsePawnControlRotation=false;Cat->Arm->SetRelativeRotation(FRotator(-18,90,0));Cat->Arm->TargetArmLength=420;}
  }
  if(Review==TEXT("residents")||Review==TEXT("caught")){
   StartRound(false);Practice=Review==TEXT("residents");
   if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){
    Cat->SetActorLocation(FVector(-400,400,40));Cat->Controller->SetControlRotation(FRotator(-15,0,0));Cat->Arm->TargetArmLength=350;
    Residents[0]->SetActorLocation(FVector(-150,400,92));Residents[1]->SetActorLocation(FVector(0,530,92));
    if(Review==TEXT("caught"))CatchCat(Cat);
   }
  }

  if(Review==TEXT("collection"))SelectCat(3);
  if(Review==TEXT("lab")||Review==TEXT("fusion")||Review==TEXT("hidden")){
   if(Review==TEXT("hidden")){SelectedCat=0;BeginFusionChoice();PartnerCat=3;}
   else{BeginFusionChoice();PartnerCat=1;if(Review==TEXT("fusion")){ConfirmSelection();Remaining=400;}}
  }
  if(Review==TEXT("hud")||Review==TEXT("pause")||Review==TEXT("results")||Review==TEXT("empty")||Review==TEXT("mvp")){
   Practice=Review!=TEXT("results")&&Review!=TEXT("empty")&&Review!=TEXT("mvp");StartRound(false);
   if(Review==TEXT("results")||Review==TEXT("mvp")){for(auto P:Props)if(P->PropId==0||P->PropId==13||P->PropId==23)Affect(P,NewChain(false));FinishRound();}
   if(Review==TEXT("empty"))FinishRound();
   if(Review==TEXT("results")||Review==TEXT("empty"))StopMeme();
   if(Review==TEXT("pause")){UGameplayStatics::SetGamePaused(GetWorld(),true);SetMenuInput(true);}
  }
 }
#endif
}
void ALabGameMode::SetMenuInput(bool Enabled){if(auto PC=GetWorld()->GetFirstPlayerController()){PC->bShowMouseCursor=Enabled;PC->bEnableClickEvents=Enabled;if(Enabled){FInputModeGameAndUI Mode;Mode.SetHideCursorDuringCapture(false);PC->SetInputMode(Mode);}else PC->SetInputMode(FInputModeGameOnly());}}
bool ALabGameMode::BounceRule() const{return Playing()&&CandySpilled;}
bool ALabGameMode::ResonanceRule() const{return Playing()&&(CurrentTraits()&CatTrait::Loud);}
void ALabGameMode::OpenSelection(){
 ClearTitle();
 ClearMeme();
 UGameplayStatics::SetGlobalTimeDilation(GetWorld(),1);SlowUntil=0;ClearDiscoveries();
 for(auto A:PresentationCats)if(IsValid(A))A->Destroy();PresentationCats.Empty();for(auto P:Props)if(IsValid(P))P->Destroy();Props.Empty();PetCamera=nullptr;
 Chains.Empty();NextChain=1;Score=Best=Current=Swipes=UsedPounces=0;Remaining=180;Pounces=3;Stage=4;RecipeId=INDEX_NONE;ResetParty();PopUntil=0;Challenge=false;Quip.Empty();HideHUD=false;SetMenuInput(true);
 if(!Unlocked(SelectedCat))SelectedCat=0;
 if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->ResetCat();Cat->ApplyIdentity(SelectedCat,false);}
}
void ALabGameMode::SelectCat(int32 Index){
 if(!Unlocked(Index))return;
 if(Stage==4){SelectedCat=Index;if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0)))Cat->ApplyIdentity(Index,false);}
 else if(Stage==5&&Index!=SelectedCat)PartnerCat=Index;
}
void ALabGameMode::ConfirmSelection(){
 if(Stage==6){StartFromTitle();return;}
 if(Stage==4){if(Progress&&FMath::CountBits((uint32)Progress->UnlockedMask)>1)BeginFusionChoice();else StartRound(false);}
 else if(Stage==5){const int R=LabCatalog::RecipeFor(SelectedCat,PartnerCat);if(!Unlocked(PartnerCat)||!RecipeUnlocked(R))return;RecipeId=R;Progress->RecipesMask|=1<<R;SaveProgress();Stage=1;Remaining=4;WorldSound(TEXT("Fusion"),.5f);SetMenuInput(false);UpdatePresentation();}
 else if(Stage==3)Lab();
}
void ALabGameMode::BeginFusionChoice(){Stage=5;Remaining=0;PartnerCat=INDEX_NONE;for(int I=0;I<LabCatalog::CatCount;I++)if(I!=SelectedCat&&Unlocked(I)){PartnerCat=I;break;}SetMenuInput(true);if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->Charging=false;Cat->Pushing=false;Cat->ZoomTime=0;Cat->GetCharacterMovement()->StopMovementImmediately();}}
void ALabGameMode::UpdatePresentation(){for(auto A:PresentationCats)if(IsValid(A))A->Destroy();PresentationCats.Empty();if(Stage!=1)return;auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));if(!Cat)return;for(int i=0;i<2;i++){const auto& D=LabCatalog::Cat(i?PartnerCat:SelectedCat);auto M=LoadObject<UStaticMesh>(nullptr,D.Mesh);if(!M)continue;auto A=GetWorld()->SpawnActor<AStaticMeshActor>();auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(M);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetWorldScale3D(FVector(55.f/FMath::Max(1.f,M->GetBoundingBox().GetSize().Z)));PresentationCats.Add(A);}}
void ALabGameMode::TryResonate(ALabProp* Source,int32 Id,FVector Direction){
 if(!ResonanceRule()||!Source)return;auto Chain=Chains.Find(Id);if(!Chain||Chain->Resonated||!Active(Id))return;Chain->Resonated=true;
 TArray<ALabProp*> Candidates;for(auto P:Props)if(IsValid(P)&&P!=Source&&!P->Heavy&&!Chain->Members.Contains(P->PropId)&&FVector::DistSquared(P->GetActorLocation(),Source->GetActorLocation())<FMath::Square(145.f))Candidates.Add(P);
 Candidates.Sort([&](const ALabProp& A,const ALabProp& B){return FVector::DistSquared(A.GetActorLocation(),Source->GetActorLocation())<FVector::DistSquared(B.GetActorLocation(),Source->GetActorLocation());});
 for(int I=0;I<FMath::Min(3,Candidates.Num());I++){auto P=Candidates[I];P->ChainId=Id;Affect(P,Id);P->Body->AddImpulse((P->GetActorLocation()-Source->GetActorLocation()).GetSafeNormal()*350+FVector(0,0,120),NAME_None,true);}
}
static void Furniture(UWorld* W,FVector Pos,FVector Scale,FLinearColor Color,const TCHAR* Mesh=TEXT("Cube")){
 auto A=W->SpawnActor<AStaticMeshActor>(Pos,FRotator::ZeroRotator);auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);auto Rounded=FString(Mesh)==TEXT("Cube")&&Scale.Z>.15f&&Scale.X<10&&Scale.Y<10?ArtShape(TEXT("SM_RoundedBlock")):nullptr;C->SetStaticMesh(Rounded?Rounded:Shape(Mesh));C->SetWorldScale3D(Scale);C->SetCollisionProfileName(TEXT("BlockAll"));Paint(C,Color);
}
static AStaticMeshActor* FabPiece(UWorld* W,const TCHAR* Group,const TCHAR* Name,FVector Pos,float Scale=1,float Yaw=0){
 auto Mesh=LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/FabLocal/%s/%s"),Group,Name));if(!Mesh)return nullptr;
 auto A=W->SpawnActor<AStaticMeshActor>(Pos,FRotator(0,Yaw,0));auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(Mesh);C->SetWorldScale3D(FVector(Scale));C->SetCollisionProfileName(TEXT("BlockAll"));return A;
}
void ALabGameMode::BuildRoom(){
 auto W=GetWorld();
 // Solid collision shell is independent of bevelled art and imported mesh collision.
 auto Barrier=[&](FVector P,FVector Ext){auto A=W->SpawnActor<AActor>();auto B=NewObject<UBoxComponent>(A);A->SetRootComponent(B);B->SetBoxExtent(Ext);B->SetCollisionProfileName(TEXT("BlockAll"));B->SetHiddenInGame(true);B->RegisterComponent();A->SetActorLocation(P);A->Tags.Add(TEXT("RoomSafety"));};
 Barrier(FVector(0,0,-30),FVector(920,720,30));
 Barrier(FVector(900,0,450),FVector(10,720,510));Barrier(FVector(-900,0,450),FVector(10,720,510));
 Barrier(FVector(0,700,450),FVector(920,10,510));Barrier(FVector(0,-700,450),FVector(920,10,510));
 const FLinearColor Wood(.49,.31,.17),Wall(.78,.74,.65);
 Furniture(W,FVector(0,0,-12),FVector(18,14,.24),Wood);
 for(int X=-850;X<900;X+=100)Furniture(W,FVector(X,0,.2),FVector(.015,14,.006),FLinearColor(.3,.19,.11));
 Furniture(W,FVector(900,0,200),FVector(.2,14,4),Wall);Furniture(W,FVector(-900,0,200),FVector(.2,14,4),Wall);
 Furniture(W,FVector(0,-700,200),FVector(18,.2,4),Blue);Furniture(W,FVector(0,700,200),FVector(18,.2,4),Wall);
 Furniture(W,FVector(0,-110,1),FVector(5.8,4.2,.015),FLinearColor(.12,.24,.26));
 FabPiece(W,TEXT("Furniture"),TEXT("SM_Coffe_Table_2"),FVector(10,0,-6));
 FabPiece(W,TEXT("Furniture"),TEXT("SM_Sofa_41"),FVector(-60,-260,0));
 FabPiece(W,TEXT("Furniture"),TEXT("SM_Commode_7"),FVector(260,-220,0),.48f);
 FabPiece(W,TEXT("Furniture"),TEXT("SM_Computer_desk_7"),FVector(260,235,0),.65f);
 FabPiece(W,TEXT("Furniture"),TEXT("SM_Chair_13"),FVector(-285,-100,0),1,90);
 FabPiece(W,TEXT("Furniture"),TEXT("SM_Chair_28"),FVector(280,360,0),1,180);
 for(int I=0;I<5;I++)FabPiece(W,TEXT("Furniture"),TEXT("SM_Prefabricated_kitchen_16"),FVector(650,-380+I*100,0),1,90);
 FabPiece(W,TEXT("Furniture"),TEXT("SM_Refrigerato_4"),FVector(650,200,0),1,90);
 FabPiece(W,TEXT("Furniture"),TEXT("SM_Wardrobe_9"),FVector(-650,560,0),1,180);
 FabPiece(W,TEXT("Furniture"),TEXT("SM_Bed_8"),FVector(-600,360,0),.9f);
 // Low steps, elevated landing, and a broad overhead catwalk form a complete return route.
 for(int I=0;I<8;I++){float Top=35*(I+1);Furniture(W,FVector(-700+I*95,-555,Top*.5f),FVector(.95,1.65,Top/100),I%2?Cream:Wood);}
 Furniture(W,FVector(400,-555,270),FVector(7,1.65,.2),Wood);
 Furniture(W,FVector(600,-300,270),FVector(1.3,3.5,.2),Wood);
 Furniture(W,FVector(600,-35,220),FVector(1.3,1.4,.16),Cream);
 // Landing targets make both ascent and diving useful, not just decorative height.
 for(int I=0;I<4;I++)FabPiece(W,TEXT("Decor"),TEXT("SM_SD_PottedPlant_01"),FVector(I%2?790:-790,I<2?-590:590,0));
 for(int I=0;I<5;I++)FabPiece(W,TEXT("Decor"),TEXT("SM_SD_LinenCurtain_01"),FVector(-400+I*160,680,80),1.3f);
 for(int I=0;I<3;I++){FabPiece(W,TEXT("Decor"),TEXT("SM_SD_PictureFrame_01"),FVector(880,-350+I*190,175),1.5f,90);FabPiece(W,TEXT("Decor"),TEXT("SM_SD_BookStack_01"),FVector(620,-300+I*110,115));}
 FabPiece(W,TEXT("Decor"),TEXT("SM_SD_TableLamp_01"),FVector(-650,550,235));
 FabPiece(W,TEXT("Decor"),TEXT("SM_SD_WallClock_01"),FVector(0,680,280),2,180);
 for(int I=0;I<4;I++){auto Fill=W->SpawnActor<APointLight>(FVector(I%2?500:-500,I<2?-350:350,310),FRotator::ZeroRotator);auto Light=Cast<UPointLightComponent>(Fill->GetLightComponent());Light->SetIntensity(95000);Light->SetAttenuationRadius(1600);Light->SetLightColor(FLinearColor(1,.91,.76));Light->SetCastShadows(false);}
 auto L=W->SpawnActor<ADirectionalLight>(FVector(0,0,300),FRotator(-55,-35,0));L->GetLightComponent()->SetIntensity(6);L->GetLightComponent()->SetLightColor(FLinearColor(1,.94,.86));
 auto Fill=W->SpawnActor<ADirectionalLight>(FVector(0,0,300),FRotator(-35,145,0));Fill->GetLightComponent()->SetIntensity(4);Fill->GetLightComponent()->SetCastShadows(false);
 auto Sky=W->SpawnActor<ASkyLight>();Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);Sky->GetLightComponent()->SetIntensity(1.5);Sky->GetLightComponent()->RecaptureSky();
 BuildCozyCorner();
 for(int I=0;I<2;I++){auto R=W->SpawnActor<ALabResident>(FVector(I?550:-550,400,92),FRotator::ZeroRotator);R->Configure(I==1);Residents.Add(R);}
}
void ALabGameMode::SpawnProps(bool Bounce){
 const FVector Positions[]={ {-55,0,57},{-10,0,52},{35,0,57},{35,0,82},{35,0,107},{-40,43,51},{130,-220,65},{190,-220,68},{220,-220,68},{250,-220,68},{290,-220,63},{330,-220,65},{165,235,61},{215,235,80},{265,235,68},{280,235,105},{330,235,64},{360,235,64}};
 const FVector Scales[]={ {.22,.22,.28},{.45,.42,.12},{.25,.25,.25},{.25,.25,.25},{.25,.25,.25},{.3,.12,.08},{.3,.3,.3},{.15,.35,.42},{.15,.35,.42},{.15,.35,.42},{.28,.28,.3},{.3,.3,.35},{.42,.3,.22},{.18,.18,.65},{.18,.42,.38},{.12,.85,.65},{.12,.25,.3},{.22,.22,.28}};
 for(int i=0;i<18;i++){auto P=GetWorld()->SpawnActor<ALabProp>(Positions[i],FRotator::ZeroRotator);P->Configure(i,Scales[i],i%3==0?Orange:i%3==1?Cream:Blue,i==6||i==11,i==15,Bounce);Props.Add(P);}
}
void ALabGameMode::StartRound(bool Fused){
 ClearTitle();
 ClearMeme();
 UGameplayStatics::SetGlobalTimeDilation(GetWorld(),1);SlowUntil=0;ClearDiscoveries();for(auto P:Props)if(IsValid(P))P->Destroy();Props.Empty();Chains.Empty();Current=0;NextChain=1;
 Stage=Fused?2:0;Remaining=180;Score=Best=Swipes=UsedPounces=0;Pounces=3;Challenge=false;Quip.Empty();QuipUntil=0;PopUntil=0;Recorded=false;
 for(auto A:PresentationCats)if(IsValid(A))A->Destroy();PresentationCats.Empty();EndKeyPlayed=false;ChaseMusicUntil=0;ResetParty();for(auto R:Residents)if(IsValid(R))R->ResetResident();SpawnProps(false);SpawnPartyProps();SpawnCozyProps();for(int I=0;I<12;I++){FVector Pos=I<6?FVector(150+I*90,-555,305):FVector(620,-350+(I-6)*90,142);auto P=GetWorld()->SpawnActor<ALabProp>(Pos,FRotator::ZeroRotator);P->Configure(200+I,FVector(.3,.3,.36),I%2?Cream:Orange,false,false,false);if(auto M=I<6?LoadObject<UStaticMesh>(nullptr,TEXT("/Game/FabLocal/Decor/SM_SD_CeramicVase_01")):ArtShape(TEXT("SM_Cup"))){P->Body->SetSimulatePhysics(false);P->Body->SetStaticMesh(M);if(I<6){P->Body->SetMaterial(0,M->GetMaterial(0));P->Body->SetWorldScale3D(FVector(36.f/FMath::Max(1.f,M->GetBoundingBox().GetSize().Z)));P->SetActorLocation(FVector(Pos.X,Pos.Y,282));}else P->Body->SetWorldScale3D(FVector(.22,.22,.28));P->Body->SetSimulatePhysics(true);P->Body->SetMassOverrideInKg(NAME_None,1.5f);P->InitialPosition=P->GetActorLocation();}P->ItemName=I<6?TEXT("高台花瓶"):TEXT("厨房瓷杯");P->BaseScore=I<6?129:69;Props.Add(P);}SetMenuInput(false);
 if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->ResetCat();Cat->SetMutation(Fused);Cat->SetActorLocation(FVector(-190,0,35*Cat->BodyScale),false,nullptr,ETeleportType::TeleportPhysics);}
}
void ALabGameMode::Retry(bool Full){if(Stage==4||Stage==6)return;if(Full)RecipeId=INDEX_NONE;StartRound(RecipeId!=INDEX_NONE);}
int32 ALabGameMode::NewChain(bool Big){FLabChain C;C.Id=NextChain++;C.Pounce=Big;C.Last=GetWorld()->GetTimeSeconds();Chains.Add(C.Id,C);return C.Id;}
bool ALabGameMode::Active(int32 Id) const {const auto C=Chains.Find(Id);return C&&GetWorld()->GetTimeSeconds()-C->Last<=2.5;}
void ALabGameMode::Affect(ALabProp* P,int32 Id){
 auto C=Chains.Find(Id);if(!P||!Playing()||!C||!Active(Id)||C->Members.Contains(P->PropId))return;
 C->Members.Add(P->PropId);C->Last=GetWorld()->GetTimeSeconds();Current=C->Members.Num();Best=FMath::Max(Best,Current);
 if(P->Scored)return;P->Scored=true;Score+=P->BaseScore;WorldSound(P->Heavy?TEXT("Landing"):TEXT("Glass"),P->Heavy?.45f:.35f);
 PopText=FString::Printf(TEXT("%s  +¥%d"),*P->ItemName,P->BaseScore);PopLocation=P->GetActorLocation();PopUntil=C->Last+1.6;
 if(P->PropId==30)DisableCamera();
 if(P->PropId==19)SpillCandy(P->GetActorLocation(),Id);
 if(P->PropId==18)Discover(1,P->GetActorLocation()+FVector(0,0,30));
 if(P->PropId==10)Discover(2,P->GetActorLocation()+FVector(0,0,30));
 if(P->PropId==22)Discover(3,P->GetActorLocation()+FVector(0,0,30));
 if(P->Heavy){SlowUntil=FPlatformTime::Seconds()+.18;UGameplayStatics::SetGlobalTimeDilation(GetWorld(),.35f);}
}
void ALabGameMode::FinishRound(){
 if(!Playing())return;
 UGameplayStatics::SetGlobalTimeDilation(GetWorld(),1);SlowUntil=0;Chains.Empty();Current=0;for(auto P:Props)if(IsValid(P))P->Body->SetSimulatePhysics(false);Stage=3;Remaining=0;Learned=true;SetMenuInput(true);RecordRun();CameraCaption=TEXT("派对结束。账单来了。");StartMeme();ReceiptGreetingPending=false;
}
void ALabGameMode::Tick(float Dt){
 Super::Tick(Dt);
 TickTitle(Dt);TickMeme(Dt);UpdateMusic();
 if(SlowUntil>0&&FPlatformTime::Seconds()>=SlowUntil){SlowUntil=0;UGameplayStatics::SetGlobalTimeDilation(GetWorld(),1);}
 if(Playing()){
  if(!Practice&&!FParse::Param(FCommandLine::Get(),TEXT("CatArtReview")))Remaining=FMath::Max(0.f,Remaining-Dt);
  Current=0;for(auto& Pair:Chains)if(Active(Pair.Key))Current=FMath::Max(Current,Pair.Value.Members.Num());
  if(!Practice&&Remaining<5&&!EndKeyPlayed){EndKeyPlayed=true;WorldSound(TEXT("Key"),.65f);}
  TickCamera(Dt);CollectDiscoveries();if(!Practice&&Remaining<=0)FinishRound();
 }else if(Stage==1){Remaining-=Dt;auto Cat=UGameplayStatics::GetPlayerPawn(GetWorld(),0);if(Cat)for(int I=0;I<PresentationCats.Num();I++){auto A=PresentationCats[I];A->SetActorLocation(Cat->GetActorLocation()+FVector(90,(I?1.f:-1.f)*FMath::Max(0.f,Remaining-1)*28,40));A->SetActorRotation(FRotator(0,Remaining*90,0));}if(Remaining<=0)StartRound(true);}
}

void ALabCat::ToggleMusic(){if(auto G=GM(GetWorld())){G->MusicMuted=!G->MusicMuted;G->UpdateMusic();UE_LOG(LogTemp,Display,TEXT("Menu music: %s, component: %s"),G->MusicMuted?TEXT("muted"):TEXT("on"),IsValid(G->Music)&&G->Music->IsPlaying()?TEXT("playing"):TEXT("unavailable"));}}

void ALabCat::RecoverToSafety(){
 auto Move=GetCharacterMovement();Move->StopMovementImmediately();Move->ClearAccumulatedForces();Move->SetMovementMode(MOVE_Walking);
 Charging=Pushing=false;ZoomTime=Charge=ActionTime=FallSpeed=0;ConsumeMovementInputVector();
 FVector Safe=LastSafePosition;if(Safe.ContainsNaN()||FMath::Abs(Safe.X)>820||FMath::Abs(Safe.Y)>620||Safe.Z<0||Safe.Z>450)Safe=FVector(-220,0,30*BodyScale+8);
 Safe.Z+=8;SetActorLocation(Safe,false,nullptr,ETeleportType::TeleportPhysics);
 Arm->bEnableCameraLag=false;Arm->TickComponent(0,LEVELTICK_All,nullptr);Arm->bEnableCameraLag=true;
 if(auto PC=Cast<APlayerController>(Controller))PC->SetViewTarget(this);
 UE_LOG(LogTemp,Display,TEXT("Cat safety recovery to %s"),*Safe.ToString());
}
void ALabCat::UpdateSafety(){
 auto G=GM(GetWorld());if(!G||!G->Playing())return;const FVector P=GetActorLocation();
 if(P.ContainsNaN()||P.Z< -40||P.Z>850||FMath::Abs(P.X)>910||FMath::Abs(P.Y)>710){RecoverToSafety();return;}
 if(GetCharacterMovement()->IsMovingOnGround()&&FMath::Abs(P.X)<800&&FMath::Abs(P.Y)<600&&P.Z>20&&P.Z<400)LastSafePosition=P;
}
void ALabCat::FellOutOfWorld(const UDamageType&){RecoverToSafety();}

void ALabCat::CalcCamera(float Dt,FMinimalViewInfo& Out){
 Super::CalcCamera(Dt,Out);auto G=GM(GetWorld());if(!G||!G->Playing())return;
 const FVector P=GetActorLocation(),Focus=P+FVector(0,0,30*BodyScale);
 if(FVector::Dist(Out.Location,P)>=150*BodyScale)return;
 FCollisionQueryParams Q;Q.AddIgnoredActor(this);FVector Best=Out.Location;float BestDistance=FVector::DistSquared(Best,Focus);
 const float SX=P.X>0?-1.f:1.f,SY=P.Y>0?-1.f:1.f;
 for(FVector Offset:{FVector(0,0,280),FVector(220*SX,0,210),FVector(0,220*SY,210),FVector(160*SX,160*SY,260)}){
  FVector Candidate=Focus+Offset;Candidate.X=FMath::Clamp(Candidate.X,-840.f,840.f);Candidate.Y=FMath::Clamp(Candidate.Y,-640.f,640.f);
  FHitResult Hit;FCollisionQueryParams Trace=Q;
  for(int Try=0;Try<5;Try++){
   if(!GetWorld()->SweepSingleByChannel(Hit,Focus,Candidate,FQuat::Identity,ECC_Camera,FCollisionShape::MakeSphere(12),Trace))break;
   if(Hit.bStartPenetrating&&Hit.GetActor()&&!Hit.GetActor()->ActorHasTag(TEXT("RoomSafety"))&&Try<4){Trace.AddIgnoredActor(Hit.GetActor());continue;}
   Candidate=Hit.Location;break;
  }
  float D=FVector::DistSquared(Candidate,Focus);if(D>BestDistance){BestDistance=D;Best=Candidate;}
 }
 Out.Location=Best;Out.Rotation=(Focus-Best).Rotation();
}
