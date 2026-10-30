#include "LabGame.h"
#include "LabCatalog.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundBase.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
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
 PrimaryActorTick.bCanEverTick=true;GetCapsuleComponent()->InitCapsuleSize(20,30);GetCharacterMovement()->MaxWalkSpeed=290;GetCharacterMovement()->JumpZVelocity=390;GetCharacterMovement()->AirControl=.6f;GetCharacterMovement()->bOrientRotationToMovement=true;GetCharacterMovement()->RotationRate=FRotator(0,800,0);bUseControllerRotationYaw=false;
 Arm=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));Arm->SetupAttachment(RootComponent);Arm->TargetArmLength=430;Arm->SocketOffset=FVector::ZeroVector;Arm->bUsePawnControlRotation=true;Arm->bEnableCameraLag=true;Arm->CameraLagSpeed=10;
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
void ALabCat::SetupPlayerInputComponent(UInputComponent* I){Super::SetupPlayerInputComponent(I);I->BindAxis("Forward",this,&ALabCat::Forward);I->BindAxis("Right",this,&ALabCat::Right);I->BindAxis("Turn",this,&ALabCat::Turn);I->BindAxis("Look",this,&ALabCat::Look);I->BindAction("Attack",IE_Pressed,this,&ALabCat::Press);I->BindAction("Attack",IE_Released,this,&ALabCat::Release);I->BindAction("Jump",IE_Pressed,this,&ALabCat::Hop);I->BindAction("Retry",IE_Pressed,this,&ALabCat::Retry).bExecuteWhenPaused=true;I->BindAction("Pause",IE_Pressed,this,&ALabCat::Pause).bExecuteWhenPaused=true;I->BindAction("HUD",IE_Pressed,this,&ALabCat::ToggleHUD);I->BindAction("Camera",IE_Pressed,this,&ALabCat::ToggleCamera);I->BindAction("Tutorial",IE_Pressed,this,&ALabCat::Tutorial).bExecuteWhenPaused=true;I->BindAction("One",IE_Pressed,this,&ALabCat::PickOne);I->BindAction("Two",IE_Pressed,this,&ALabCat::PickTwo);I->BindAction("Three",IE_Pressed,this,&ALabCat::PickThree);I->BindAction("Four",IE_Pressed,this,&ALabCat::PickFour);I->BindAction("Push",IE_Pressed,this,&ALabCat::PushStart);I->BindAction("Push",IE_Released,this,&ALabCat::PushStop);I->BindAction("Zoom",IE_Pressed,this,&ALabCat::ZoomStart);I->BindAction("Zoom",IE_Released,this,&ALabCat::ZoomStop);I->BindAction("Meow",IE_Pressed,this,&ALabCat::Meow);I->BindAction("Shake",IE_Pressed,this,&ALabCat::ToggleShake);I->BindAction("Practice",IE_Pressed,this,&ALabCat::TogglePractice);I->BindAction("Confirm",IE_Pressed,this,&ALabCat::ConfirmSelection);I->BindAction("Select",IE_Pressed,this,&ALabCat::OpenSelection).bExecuteWhenPaused=true;}
void ALabCat::Forward(float V){auto G=GM(GetWorld());if(G&&G->Playing()){if(ZoomTime<=0)AddMovementInput(FRotator(0,GetControlRotation().Yaw,0).Vector(),V);if(V!=0)Idle=0;}}
void ALabCat::Right(float V){auto G=GM(GetWorld());if(G&&G->Playing()){if(ZoomTime<=0)AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V);if(V!=0)Idle=0;}}
void ALabCat::Turn(float V){if(!FixedCamera&&GM(GetWorld())&&GM(GetWorld())->Playing())AddControllerYawInput(FMath::Clamp(V,-3.f,3.f));}
void ALabCat::Look(float V){if(!FixedCamera&&GM(GetWorld())&&GM(GetWorld())->Playing())AddControllerPitchInput(FMath::Clamp(V,-3.f,3.f));}
void ALabCat::TogglePractice(){if(auto G=GM(GetWorld()))G->SetPractice();}
void ALabCat::Press(){auto G=GM(GetWorld());if(G&&G->Playing()&&!UGameplayStatics::IsGamePaused(GetWorld())){Charging=true;Innocent=false;Pushing=false;Charge=0;Idle=0;}}
void ALabCat::Release(){
 auto G=GM(GetWorld());if(!Charging||!G||!G->Playing())return;Charging=false;
 const bool Pounce=Charge>=.25f;const bool Super=Pounce&&Charge>=1.05f&&G->Pounces>0;
 const int32 Chain=G->NewChain(Pounce);G->Swipes++;if(Super){G->Pounces--;G->UsedPounces++;}
 FVector Dir=AttackDirection();auto Target=FindTarget(Pounce?235:150,.10f);
 if(Target){Dir=(Target->GetActorLocation()-GetActorLocation()).GetSafeNormal2D();}
 SetActorRotation(Dir.Rotation());ActionPounce=Pounce;ActionTime=Pounce?.43f:.36f;ActiveAnimation=INDEX_NONE;Idle=0;
 const float Power=(Super?1500.f:Pounce?FMath::Lerp(620.f,1000.f,FMath::Clamp(Charge,0.f,1.f)):LabCatalog::Cat(G->SelectedCat).Swipe)*(Traits&CatTrait::Giant?1.65f:1.f);
 if(Target){Target->Strike(Chain,Dir*Power+FVector(0,0,Pounce?200:110));Impact(Target->Heavy);}
 if(Pounce)LaunchCharacter(Dir*(Super?1050:FMath::Lerp(480.f,780.f,FMath::Clamp(Charge,0.f,1.f)))+FVector(0,0,Super?260:180),true,true);
}
void ALabCat::Hop(){if(auto G=GM(GetWorld());G&&G->Playing()){Jump();Innocent=false;Idle=0;}}
void ALabCat::Retry(){UGameplayStatics::SetGamePaused(GetWorld(),false);if(auto G=GM(GetWorld()))G->Retry();}
void ALabCat::Tutorial(){UGameplayStatics::SetGamePaused(GetWorld(),false);if(auto G=GM(GetWorld()))G->Retry(true);}
void ALabCat::Pause(){auto G=GM(GetWorld());if(!G||G->Stage==4||G->Stage==5)return;Charging=false;Pushing=false;ZoomTime=0;Charge=0;UGameplayStatics::SetGamePaused(GetWorld(),!UGameplayStatics::IsGamePaused(GetWorld()));}
void ALabCat::ToggleHUD(){if(auto G=GM(GetWorld()))G->HideHUD=!G->HideHUD;}
void ALabCat::ToggleCamera(){FixedCamera=!FixedCamera;Arm->bUsePawnControlRotation=!FixedCamera;Arm->SetRelativeRotation(FRotator(-40,-30,0));Arm->TargetArmLength=FixedCamera?650:430;}
void ALabCat::ResetCat(){if(auto PC=Cast<APlayerController>(Controller)){PC->PlayerCameraManager->ViewPitchMin=-65;PC->PlayerCameraManager->ViewPitchMax=-10;PC->FlushPressedKeys();}SetActorLocation(FVector(-220,0,35),false,nullptr,ETeleportType::TeleportPhysics);GetCharacterMovement()->StopMovementImmediately();Charging=false;Pushing=false;ZoomTime=0;ZoomCooldown=0;MeowCooldown=0;AutoMeowCooldown=0;PushClock=0;FallSpeed=0;Charge=0;Idle=0;ActionTime=0;ImpactTime=0;Innocent=false;ActionPounce=false;Arm->SocketOffset=FVector::ZeroVector;SetActorRotation(FRotator::ZeroRotator);if(Controller)Controller->SetControlRotation(FRotator(-32,0,0));}
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
 TickInstincts(Dt);Idle+=Dt;ActionTime=FMath::Max(0.f,ActionTime-Dt);
 const float Speed=GetVelocity().Size2D(),T=GetWorld()->GetTimeSeconds();
 if(Speed>12||Charging||Pushing||ZoomTime>0)Idle=0;
 Innocent=G->Playing()&&Idle>.85f&&Speed<12&&!GetCharacterMovement()->IsFalling();
 if(Charging){Charge=FMath::Min(1.25f,Charge+Dt);DrawDebugDirectionalArrow(GetWorld(),GetActorLocation(),GetActorLocation()+AttackDirection()*(90+Charge*110),20,Charge>=1.05f&&G->Pounces>0?FColor::Yellow:FColor(255,106,43),false,0,0,3);}
 if(GetMesh()->GetSkeletalMeshAsset()){
  const int Desired=Charging?4:Pushing?6:ActionTime>0?(ActionPounce?7:5):Innocent?3:Speed>180?1:Speed>15?2:0;
  if(Desired!=ActiveAnimation&&Locomotion.IsValidIndex(Desired)&&Locomotion[Desired]){GetMesh()->PlayAnimation(Locomotion[Desired],Desired!=5&&Desired!=7);ActiveAnimation=Desired;}
  const float Wiggle=Charging?FMath::Sin(T*(15+Charge*12))*FMath::Lerp(2.f,6.f,Charge/1.25f):0;
  GetMesh()->SetRelativeRotation(FRotator(0,-90+Wiggle,0));
  GetMesh()->SetRelativeScale3D(FVector(MeshScale));
  GetMesh()->SetRelativeLocation(FVector(0,0,MeshFloor));
 }
 Visual->SetRelativeRotation(FRotator(ActionTime>0?-18:Innocent?12:0,0,0));
 if(Shake&&ImpactTime>0){ImpactTime=FMath::Max(0.f,ImpactTime-Dt);Arm->SocketOffset=FVector(0,FMath::Sin(T*110)*ImpactTime*32,FMath::Sin(T*87)*ImpactTime*20);}else Arm->SocketOffset=FVector::ZeroVector;
 if(GetActorLocation().Z< -100)ResetCat();
}
ALabGameMode::ALabGameMode(){PrimaryActorTick.bCanEverTick=true;DefaultPawnClass=ALabCat::StaticClass();HUDClass=ALabHUD::StaticClass();}
void ALabGameMode::BeginPlay(){Super::BeginPlay();LoadProgress();BuildRoom();OpenSelection();
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("CatArtReview"))){int32 ReviewCat=0,ReviewRecipe=-1;FParse::Value(FCommandLine::Get(),TEXT("ReviewCat="),ReviewCat);FParse::Value(FCommandLine::Get(),TEXT("ReviewRecipe="),ReviewRecipe);SelectedCat=FMath::Clamp(ReviewCat,0,3);RecipeId=FMath::Clamp(ReviewRecipe,-1,5);StartRound(RecipeId>=0);if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->FixedCamera=true;Cat->Arm->bUsePawnControlRotation=false;Cat->Arm->SetRelativeRotation(FRotator(-25,-25,0));Cat->Arm->TargetArmLength=260;Cat->SetActorRotation(FRotator(0,155,0));}}
#endif
}
void ALabGameMode::SetMenuInput(bool Enabled){if(auto PC=GetWorld()->GetFirstPlayerController()){PC->bShowMouseCursor=Enabled;PC->bEnableClickEvents=Enabled;if(Enabled){FInputModeGameAndUI Mode;Mode.SetHideCursorDuringCapture(false);PC->SetInputMode(Mode);}else PC->SetInputMode(FInputModeGameOnly());}}
bool ALabGameMode::BounceRule() const{return Playing()&&CandySpilled;}
bool ALabGameMode::ResonanceRule() const{return Playing()&&(CurrentTraits()&CatTrait::Loud);}
void ALabGameMode::OpenSelection(){
 UGameplayStatics::SetGlobalTimeDilation(GetWorld(),1);SlowUntil=0;ClearDiscoveries();
 for(auto A:PresentationCats)if(IsValid(A))A->Destroy();PresentationCats.Empty();for(auto P:Props)if(IsValid(P))P->Destroy();Props.Empty();PetCamera=nullptr;
 Chains.Empty();NextChain=1;Score=Best=Current=Swipes=UsedPounces=0;Remaining=60;Pounces=3;Stage=4;RecipeId=INDEX_NONE;ResetParty();PopUntil=0;Challenge=false;Quip.Empty();HideHUD=false;SetMenuInput(true);
 if(!Unlocked(SelectedCat))SelectedCat=0;
 if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->ResetCat();Cat->ApplyIdentity(SelectedCat,false);}
}
void ALabGameMode::SelectCat(int32 Index){
 if(!Unlocked(Index))return;
 if(Stage==4){SelectedCat=Index;if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0)))Cat->ApplyIdentity(Index,false);}
 else if(Stage==5&&Index!=SelectedCat)PartnerCat=Index;
}
void ALabGameMode::ConfirmSelection(){
 if(Stage==4){if(Progress&&FMath::CountBits((uint32)Progress->UnlockedMask)>1)BeginFusionChoice();else StartRound(false);}
 else if(Stage==5){const int R=LabCatalog::RecipeFor(SelectedCat,PartnerCat);if(!Unlocked(PartnerCat)||!RecipeUnlocked(R))return;RecipeId=R;Progress->RecipesMask|=1<<R;SaveProgress();Stage=1;Remaining=4;SetMenuInput(false);UpdatePresentation();}
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
 auto A=W->SpawnActor<AStaticMeshActor>(Pos,FRotator::ZeroRotator);auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);auto Rounded=FString(Mesh)==TEXT("Cube")&&Scale.Z>.15f?ArtShape(TEXT("SM_RoundedBlock")):nullptr;C->SetStaticMesh(Rounded?Rounded:Shape(Mesh));C->SetWorldScale3D(Scale);C->SetCollisionProfileName(TEXT("BlockAll"));Paint(C,Color);
}
void ALabGameMode::BuildRoom(){
 for(int i=0;i<2;i++){auto Fill=GetWorld()->SpawnActor<APointLight>(FVector(-140,i?200:-200,230),FRotator::ZeroRotator);auto Light=Cast<UPointLightComponent>(Fill->GetLightComponent());Light->SetIntensity(60000);Light->SetAttenuationRadius(1400);Light->SetLightColor(FLinearColor(1,.94,.86));Light->SetCastShadows(false);}
 Furniture(GetWorld(),FVector(0,0,-12),FVector(9,7,.24),Cream);Furniture(GetWorld(),FVector(440,0,150),FVector(.2,7,3),Cream);Furniture(GetWorld(),FVector(0,-350,150),FVector(9,.2,3),Blue);Furniture(GetWorld(),FVector(0,350,150),FVector(9,.2,3),Cream);
 Furniture(GetWorld(),FVector(10,0,35),FVector(1.8,1.25,.18),Blue);for(int i=0;i<4;i++)Furniture(GetWorld(),FVector(i<2?-60:80,i%2?45:-45,13),FVector(.12,.12,.26),Ink);
 Furniture(GetWorld(),FVector(-60,-245,30),FVector(2.6,.8,.6),Blue);Furniture(GetWorld(),FVector(-60,-285,65),FVector(2.6,.2,1),Blue);
 Furniture(GetWorld(),FVector(260,-220,40),FVector(1.9,.75,.15),Cream);Furniture(GetWorld(),FVector(260,235,40),FVector(2.3,.8,.2),Blue);
 auto L=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,300),FRotator(-55,-35,0));Cast<UDirectionalLightComponent>(L->GetLightComponent())->SetForwardShadingPriority(1);L->GetLightComponent()->SetIntensity(4);L->GetLightComponent()->SetLightColor(FLinearColor(1,.92,.8));
 auto FillSun=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,300),FRotator(-35,145,0));FillSun->GetLightComponent()->SetIntensity(1.5f);FillSun->GetLightComponent()->SetCastShadows(false);FillSun->GetLightComponent()->SetLightColor(FLinearColor(.85,.92,1));
 auto S=GetWorld()->SpawnActor<ASkyLight>();S->GetLightComponent()->SetMobility(EComponentMobility::Movable);S->GetLightComponent()->SetIntensity(1.2);S->GetLightComponent()->RecaptureSky();
}
void ALabGameMode::SpawnProps(bool Bounce){
 const FVector Positions[]={ {-55,0,57},{-10,0,52},{35,0,57},{35,0,82},{35,0,107},{-40,43,51},{130,-220,65},{190,-220,68},{220,-220,68},{250,-220,68},{290,-220,63},{330,-220,65},{165,235,61},{215,235,80},{265,235,68},{280,235,105},{330,235,64},{360,235,64}};
 const FVector Scales[]={ {.22,.22,.28},{.45,.42,.12},{.25,.25,.25},{.25,.25,.25},{.25,.25,.25},{.3,.12,.08},{.3,.3,.3},{.15,.35,.42},{.15,.35,.42},{.15,.35,.42},{.28,.28,.3},{.3,.3,.35},{.42,.3,.22},{.18,.18,.65},{.18,.42,.38},{.12,.85,.65},{.12,.25,.3},{.22,.22,.28}};
 for(int i=0;i<18;i++){auto P=GetWorld()->SpawnActor<ALabProp>(Positions[i],FRotator::ZeroRotator);P->Configure(i,Scales[i],i%3==0?Orange:i%3==1?Cream:Blue,i==6||i==11,i==15,Bounce);Props.Add(P);}
}
void ALabGameMode::StartRound(bool Fused){
 UGameplayStatics::SetGlobalTimeDilation(GetWorld(),1);SlowUntil=0;ClearDiscoveries();for(auto P:Props)if(IsValid(P))P->Destroy();Props.Empty();Chains.Empty();Current=0;NextChain=1;
 Stage=Fused?2:0;Remaining=60;Score=Best=Swipes=UsedPounces=0;Pounces=3;Challenge=false;Quip.Empty();QuipUntil=0;PopUntil=0;Recorded=false;
 for(auto A:PresentationCats)if(IsValid(A))A->Destroy();PresentationCats.Empty();ResetParty();SpawnProps(false);SpawnPartyProps();SetMenuInput(false);
 if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->ResetCat();Cat->SetMutation(Fused);Cat->SetActorLocation(FVector(-190,0,35*Cat->BodyScale),false,nullptr,ETeleportType::TeleportPhysics);}
}
void ALabGameMode::Retry(bool Full){if(Stage==4)return;if(Full)RecipeId=INDEX_NONE;StartRound(RecipeId!=INDEX_NONE);}
int32 ALabGameMode::NewChain(bool Big){FLabChain C;C.Id=NextChain++;C.Pounce=Big;C.Last=GetWorld()->GetTimeSeconds();Chains.Add(C.Id,C);return C.Id;}
bool ALabGameMode::Active(int32 Id) const {const auto C=Chains.Find(Id);return C&&GetWorld()->GetTimeSeconds()-C->Last<=2.5;}
void ALabGameMode::Affect(ALabProp* P,int32 Id){
 auto C=Chains.Find(Id);if(!P||!Playing()||!C||!Active(Id)||C->Members.Contains(P->PropId))return;
 C->Members.Add(P->PropId);C->Last=GetWorld()->GetTimeSeconds();Current=C->Members.Num();Best=FMath::Max(Best,Current);
 if(P->Scored)return;P->Scored=true;Score+=P->BaseScore;
 PopText=FString::Printf(TEXT("%s：已去世  +￥%d"),*P->ItemName,P->BaseScore);PopLocation=P->GetActorLocation();PopUntil=C->Last+1.6;
 if(P->PropId==30)DisableCamera();
 if(P->PropId==19)SpillCandy(P->GetActorLocation(),Id);
 if(P->PropId==18)Discover(1,P->GetActorLocation()+FVector(0,0,30));
 if(P->PropId==10)Discover(2,P->GetActorLocation()+FVector(0,0,30));
 if(P->PropId==22)Discover(3,P->GetActorLocation()+FVector(0,0,30));
 if(P->Heavy){SlowUntil=FPlatformTime::Seconds()+.18;UGameplayStatics::SetGlobalTimeDilation(GetWorld(),.35f);}
}
void ALabGameMode::FinishRound(){
 UGameplayStatics::SetGlobalTimeDilation(GetWorld(),1);SlowUntil=0;Chains.Empty();Current=0;for(auto P:Props)if(IsValid(P))P->Body->SetSimulatePhysics(false);Stage=3;Remaining=0;Learned=true;SetMenuInput(true);RecordRun();Speak(TEXT("OwnerHome"));CameraCaption=TEXT("门口传来钥匙声……主人到家了。");
}
void ALabGameMode::Tick(float Dt){
 Super::Tick(Dt);
 if(SlowUntil>0&&FPlatformTime::Seconds()>=SlowUntil){SlowUntil=0;UGameplayStatics::SetGlobalTimeDilation(GetWorld(),1);}
 if(Playing()){
  if(!Practice&&!FParse::Param(FCommandLine::Get(),TEXT("CatArtReview")))Remaining=FMath::Max(0.f,Remaining-Dt);
  Current=0;for(auto& Pair:Chains)if(Active(Pair.Key))Current=FMath::Max(Current,Pair.Value.Members.Num());
  TickCamera(Dt);CollectDiscoveries();if(!Practice&&Remaining<=0)FinishRound();
 }else if(Stage==1){Remaining-=Dt;auto Cat=UGameplayStatics::GetPlayerPawn(GetWorld(),0);if(Cat)for(int I=0;I<PresentationCats.Num();I++){auto A=PresentationCats[I];A->SetActorLocation(Cat->GetActorLocation()+FVector(90,(I?1.f:-1.f)*FMath::Max(0.f,Remaining-1)*28,40));A->SetActorRotation(FRotator(0,Remaining*90,0));}if(Remaining<=0)StartRound(true);}
}
