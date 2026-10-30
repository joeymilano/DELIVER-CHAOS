#include "LabGame.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
static FLinearColor Cream(0.90f,0.83f,0.72f), Ink(0.025f,0.028f,0.028f), Orange(1,0.145f,0.025f), Blue(0.43f,0.59f,0.64f);
static UStaticMesh* Shape(const TCHAR* Name){return LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Name,Name));}
static void Paint(UStaticMeshComponent* C,FLinearColor Color){
 UMaterialInterface* M=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Art/M_Palette.M_Palette"));
 if(!M) M=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
 if(M){auto D=UMaterialInstanceDynamic::Create(M,C);D->SetVectorParameterValue(TEXT("Color"),Color);C->SetMaterial(0,D);}
}
static ALabGameMode* GM(UWorld* W){return Cast<ALabGameMode>(W->GetAuthGameMode());}
ALabProp::ALabProp(){PrimaryActorTick.bCanEverTick=true; Body=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));SetRootComponent(Body);Body->SetStaticMesh(Shape(TEXT("Cube")));Body->SetCollisionProfileName(TEXT("PhysicsActor"));Body->SetNotifyRigidBodyCollision(true);Body->SetUseCCD(true);Body->OnComponentHit.AddDynamic(this,&ALabProp::Hit);}
void ALabProp::Configure(int32 Id,FVector Scale,FLinearColor Color,bool Ball,bool IsHeavy,bool Bounce){
 PropId=Id;Heavy=IsHeavy;Mutated=Bounce&&!Heavy;BaseScore=Id==15?300:(Id==11||Id==13?150:100);
 Body->SetStaticMesh(Shape(Ball?TEXT("Sphere"):TEXT("Cube")));Body->SetWorldScale3D(Scale);Paint(Body,Color);
 Physics=NewObject<UPhysicalMaterial>(this);Physics->Friction=.45f;Physics->Restitution=Mutated?.93f:.18f;Physics->bOverrideRestitutionCombineMode=true;Physics->RestitutionCombineMode=EFrictionCombineMode::Max;Body->SetPhysMaterialOverride(Physics);
 Body->SetSimulatePhysics(true);Body->SetMassOverrideInKg(NAME_None,Heavy?18.f:1.5f);Body->SetLinearDamping(Mutated?.12f:.5f);Body->SetAngularDamping(.7f);
 Body->SetPhysicsMaxAngularVelocityInDegrees(Heavy?100.f:650.f);
 if(Heavy){Body->BodyInstance.bLockXTranslation=true;Body->BodyInstance.bLockYTranslation=true;Body->BodyInstance.bLockZRotation=true;Body->BodyInstance.bLockXRotation=true;Body->BodyInstance.SetDOFLock(EDOFMode::SixDOF);}
}
void ALabProp::Strike(int32 Chain,FVector V){auto G=GM(GetWorld());if(!G||!G->Playing())return;ChainId=Chain;G->Affect(this,Chain);Body->AddImpulse(V,NAME_None,true);}
void ALabProp::Hit(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,FVector Impulse,const FHitResult& R){
 auto G=GM(GetWorld());if(!G||!G->Playing())return;
 auto P=Cast<ALabProp>(Other);
 if(P && Impulse.Size()>15){if(G->Active(ChainId)&&!G->Active(P->ChainId)){P->ChainId=ChainId;G->Affect(P,ChainId);}else if(!G->Active(ChainId)&&G->Active(P->ChainId)){ChainId=P->ChainId;G->Affect(this,ChainId);}}
 if(Mutated && G->Active(ChainId)&&Impulse.Size()>80 && Bounces<4){Bounces++;Body->AddImpulse(R.ImpactNormal*160,NAME_None,true);if(Bounces==4){Physics->Restitution=.15f;Body->SetPhysMaterialOverride(Physics);Body->SetLinearDamping(2);}}
}
void ALabProp::Tick(float Dt){Super::Tick(Dt);FVector V=Body->GetPhysicsLinearVelocity();if(V.Size()>1800)Body->SetPhysicsLinearVelocity(V.GetClampedToMaxSize(1800));if(GetActorLocation().Z< -150 || GetActorLocation().Size()>2500){Body->SetSimulatePhysics(false);SetActorHiddenInGame(true);SetActorEnableCollision(false);}}
ALabCat::ALabCat(){
 PrimaryActorTick.bCanEverTick=true;GetCapsuleComponent()->InitCapsuleSize(20,30);GetCharacterMovement()->MaxWalkSpeed=290;GetCharacterMovement()->JumpZVelocity=390;GetCharacterMovement()->AirControl=.6f;GetCharacterMovement()->bOrientRotationToMovement=true;GetCharacterMovement()->RotationRate=FRotator(0,800,0);bUseControllerRotationYaw=false;
 Arm=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));Arm->SetupAttachment(RootComponent);Arm->TargetArmLength=430;Arm->SocketOffset=FVector(0,0,110);Arm->bUsePawnControlRotation=true;Arm->bEnableCameraLag=true;Arm->CameraLagSpeed=10;
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
void ALabCat::BeginPlay(){Super::BeginPlay();TArray<UStaticMeshComponent*> Parts;GetComponents(Parts);for(auto C:Parts){FString N=C->GetName();Paint(C,N==TEXT("Patch")||N==TEXT("Tail")||N==TEXT("Nose")||N==TEXT("EyeR")||N.StartsWith(TEXT("Ear"))?Ink:Cream);}if(Controller)Controller->SetControlRotation(FRotator(-32,0,0));}
void ALabCat::SetupPlayerInputComponent(UInputComponent* I){Super::SetupPlayerInputComponent(I);I->BindAxis("Forward",this,&ALabCat::Forward);I->BindAxis("Right",this,&ALabCat::Right);I->BindAxis("Turn",this,&ALabCat::Turn);I->BindAxis("Look",this,&ALabCat::Look);I->BindAction("Attack",IE_Pressed,this,&ALabCat::Press);I->BindAction("Attack",IE_Released,this,&ALabCat::Release);I->BindAction("Jump",IE_Pressed,this,&ALabCat::Hop);I->BindAction("Retry",IE_Pressed,this,&ALabCat::Retry).bExecuteWhenPaused=true;I->BindAction("Pause",IE_Pressed,this,&ALabCat::Pause).bExecuteWhenPaused=true;I->BindAction("HUD",IE_Pressed,this,&ALabCat::ToggleHUD);I->BindAction("Camera",IE_Pressed,this,&ALabCat::ToggleCamera);I->BindAction("Tutorial",IE_Pressed,this,&ALabCat::Tutorial).bExecuteWhenPaused=true;}
void ALabCat::Forward(float V){auto G=GM(GetWorld());if(G&&G->Playing()){AddMovementInput(FRotator(0,GetControlRotation().Yaw,0).Vector(),V);if(V!=0)Idle=0;}}
void ALabCat::Right(float V){auto G=GM(GetWorld());if(G&&G->Playing()){AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V);if(V!=0)Idle=0;}}
void ALabCat::Turn(float V){if(!FixedCamera)AddControllerYawInput(V);}
void ALabCat::Look(float V){if(!FixedCamera)AddControllerPitchInput(V);}
void ALabCat::Press(){auto G=GM(GetWorld());if(G&&G->Playing()){Charging=true;Charge=0;Idle=0;}}
void ALabCat::Release(){auto G=GM(GetWorld());if(!Charging||!G||!G->Playing())return;Charging=false;bool Big=Charge>=.25f&&G->Pounces>0;int32 Chain=G->NewChain(Big);G->Swipes++;if(Big){G->Pounces--;G->UsedPounces++;}FVector Dir=FRotator(0,GetControlRotation().Yaw,0).Vector();SetActorRotation(Dir.Rotation());ActionTime=.25f;Idle=0;
 float Power=Big?FMath::Lerp(650.f,1250.f,FMath::Clamp(Charge/.8f,0.f,1.f)):420.f;
 FVector Origin=GetActorLocation()+FVector(0,0,30);TArray<FHitResult> Hits;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
 GetWorld()->SweepMultiByObjectType(Hits,Origin,Origin+Dir*(Big?170:95),FQuat::Identity,FCollisionObjectQueryParams(ECC_PhysicsBody),FCollisionShape::MakeSphere(Big?35:22),Q);
 Hits.Sort([](const FHitResult& A,const FHitResult& B){return A.Distance<B.Distance;});
 TSet<ALabProp*> HitProps;for(auto& H:Hits){auto P=Cast<ALabProp>(H.GetActor());if(P&&!HitProps.Contains(P)){HitProps.Add(P);P->Strike(Chain,Dir*Power+FVector(0,0,Big?190:100));break;}}
 if(Big)LaunchCharacter(Dir*650+FVector(0,0,180),true,true);
}
void ALabCat::Hop(){if(auto G=GM(GetWorld());G&&G->Playing()){Jump();Idle=0;}}
void ALabCat::Retry(){UGameplayStatics::SetGamePaused(GetWorld(),false);if(auto G=GM(GetWorld()))G->Retry();}
void ALabCat::Tutorial(){UGameplayStatics::SetGamePaused(GetWorld(),false);if(auto G=GM(GetWorld()))G->Retry(true);}
void ALabCat::Pause(){Charging=false;Charge=0;UGameplayStatics::SetGamePaused(GetWorld(),!UGameplayStatics::IsGamePaused(GetWorld()));}
void ALabCat::ToggleHUD(){if(auto G=GM(GetWorld()))G->HideHUD=!G->HideHUD;}
void ALabCat::ToggleCamera(){FixedCamera=!FixedCamera;Arm->bUsePawnControlRotation=!FixedCamera;Arm->SetRelativeRotation(FRotator(-40,-30,0));Arm->TargetArmLength=FixedCamera?650:430;}
void ALabCat::ResetCat(){SetActorLocation(FVector(-220,0,35),false,nullptr,ETeleportType::TeleportPhysics);GetCharacterMovement()->StopMovementImmediately();Charging=false;Charge=0;Idle=0;ActionTime=0;if(Controller)Controller->SetControlRotation(FRotator(-32,0,0));}
void ALabCat::SetMutation(bool On){for(auto P:Paws)Paint(P,On?Orange:Cream);Paint(Tail,On?Orange:Ink);}
void ALabCat::Tick(float Dt){Super::Tick(Dt);auto G=GM(GetWorld());if(!G)return;Idle+=Dt;ActionTime=FMath::Max(0.f,ActionTime-Dt);if(Charging){Charge=FMath::Min(.8f,Charge+Dt);FVector A=GetActorLocation();DrawDebugDirectionalArrow(GetWorld(),A,A+FRotator(0,GetControlRotation().Yaw,0).Vector()*(80+Charge*120),24,FColor(255,106,43),false,0,0,3);}
 Innocent=Idle>1.2f&&G->Best>=4&&G->Current==0;float Speed=GetVelocity().Size2D();float T=GetWorld()->GetTimeSeconds();Visual->SetRelativeRotation(FRotator(ActionTime>0?-18:Innocent?12:0,0,0));Visual->SetRelativeLocation(FVector(0,0,-23+(Speed>20?FMath::Sin(T*20)*2:0)));
 for(int i=0;i<Paws.Num();i++)Paws[i]->SetRelativeRotation(FRotator(Speed>20?FMath::Sin(T*20+i*PI)*28:0,0,0));Tail->SetRelativeRotation(FRotator(-30,FMath::Sin(T*3)*15,0));if(GetActorLocation().Z< -100)ResetCat();
}
ALabGameMode::ALabGameMode(){PrimaryActorTick.bCanEverTick=true;DefaultPawnClass=ALabCat::StaticClass();HUDClass=ALabHUD::StaticClass();}
void ALabGameMode::BeginPlay(){Super::BeginPlay();BuildRoom();StartRound(false);if(auto PC=GetWorld()->GetFirstPlayerController()){PC->SetInputMode(FInputModeGameOnly());PC->bShowMouseCursor=false;}}
static void Furniture(UWorld* W,FVector Pos,FVector Scale,FLinearColor Color,const TCHAR* Mesh=TEXT("Cube")){
 auto A=W->SpawnActor<AStaticMeshActor>(Pos,FRotator::ZeroRotator);auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(Shape(Mesh));C->SetWorldScale3D(Scale);C->SetCollisionProfileName(TEXT("BlockAll"));Paint(C,Color);
}
void ALabGameMode::BuildRoom(){
 for(int i=0;i<2;i++){auto Fill=GetWorld()->SpawnActor<APointLight>(FVector(-140,i?200:-200,230),FRotator::ZeroRotator);auto Light=Cast<UPointLightComponent>(Fill->GetLightComponent());Light->SetIntensity(18000);Light->SetAttenuationRadius(1400);Light->SetLightColor(FLinearColor(1,.94,.86));Light->SetCastShadows(false);}
 Furniture(GetWorld(),FVector(0,0,-12),FVector(9,7,.24),Cream);Furniture(GetWorld(),FVector(440,0,150),FVector(.2,7,3),Cream);Furniture(GetWorld(),FVector(0,-350,150),FVector(9,.2,3),Blue);Furniture(GetWorld(),FVector(0,350,150),FVector(9,.2,3),Cream);
 Furniture(GetWorld(),FVector(10,0,35),FVector(1.8,1.25,.18),Blue);for(int i=0;i<4;i++)Furniture(GetWorld(),FVector(i<2?-60:80,i%2?45:-45,13),FVector(.12,.12,.26),Ink);
 Furniture(GetWorld(),FVector(-60,-245,30),FVector(2.6,.8,.6),Blue);Furniture(GetWorld(),FVector(-60,-285,65),FVector(2.6,.2,1),Blue);
 Furniture(GetWorld(),FVector(260,-220,40),FVector(1.9,.75,.15),Cream);Furniture(GetWorld(),FVector(260,235,40),FVector(2.3,.8,.2),Blue);
 auto L=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,300),FRotator(-55,-35,0));L->GetLightComponent()->SetIntensity(4);L->GetLightComponent()->SetLightColor(FLinearColor(1,.92,.8));
 auto S=GetWorld()->SpawnActor<ASkyLight>();S->GetLightComponent()->SetMobility(EComponentMobility::Movable);S->GetLightComponent()->SetIntensity(1.2);S->GetLightComponent()->RecaptureSky();
}
void ALabGameMode::SpawnProps(bool Bounce){
 const FVector Positions[]={ {-55,0,57},{-10,0,52},{35,0,57},{35,0,82},{35,0,107},{-40,43,51},{130,-220,65},{190,-220,68},{220,-220,68},{250,-220,68},{290,-220,63},{330,-220,65},{165,235,61},{215,235,80},{265,235,68},{280,235,105},{330,235,64},{360,235,64}};
 const FVector Scales[]={ {.22,.22,.28},{.45,.42,.12},{.25,.25,.25},{.25,.25,.25},{.25,.25,.25},{.3,.12,.08},{.3,.3,.3},{.15,.35,.42},{.15,.35,.42},{.15,.35,.42},{.28,.28,.3},{.3,.3,.35},{.42,.3,.22},{.18,.18,.65},{.18,.42,.38},{.12,.85,.65},{.12,.25,.3},{.22,.22,.28}};
 for(int i=0;i<18;i++){auto P=GetWorld()->SpawnActor<ALabProp>(Positions[i],FRotator::ZeroRotator);P->Configure(i,Scales[i],i%3==0?Orange:i%3==1?Cream:Blue,i==6||i==11,i==15,Bounce);Props.Add(P);}
}
void ALabGameMode::StartRound(bool Bounce){for(auto P:Props)if(IsValid(P))P->Destroy();Props.Empty();Chains.Empty();Current=0;NextChain=1;Stage=Bounce?2:0;Remaining=Bounce?60:45;Pounces=3;Challenge=false;SpawnProps(Bounce);if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->ResetCat();Cat->SetMutation(Bounce);}}
void ALabGameMode::Retry(bool Full){Score=Best=Swipes=UsedPounces=0;Quip.Empty();if(Full)Learned=false;StartRound(Learned);}
int32 ALabGameMode::NewChain(bool Big){FLabChain C;C.Id=NextChain++;C.Pounce=Big;C.Last=GetWorld()->GetTimeSeconds();Chains.Add(C.Id,C);return C.Id;}
bool ALabGameMode::Active(int32 Id) const {const auto C=Chains.Find(Id);return C&&GetWorld()->GetTimeSeconds()-C->Last<=2.5;}
void ALabGameMode::Affect(ALabProp* P,int32 Id){auto C=Chains.Find(Id);if(!Playing()||!C||!Active(Id)||C->Members.Contains(P->PropId))return;C->Members.Add(P->PropId);C->Last=GetWorld()->GetTimeSeconds();int32 N=C->Members.Num();Current=N;Best=FMath::Max(Best,N);if(!P->Scored){P->Scored=true;Score+=P->BaseScore*FMath::Min(N,5);}if(Stage==2&&C->Pounce&&N>=5&&!Challenge){Challenge=true;Score+=500;Quip=TEXT("ONE POUNCE. FIVE VICTIMS.");QuipUntil=C->Last+3;}if(P->PropId==15){Paint(P->Body,Orange);Quip=TEXT("SIGNAL HIJACKED BY CAT.");QuipUntil=C->Last+3;}}
void ALabGameMode::FinishRound(){Score+=Pounces*150;Chains.Empty();Current=0;for(auto P:Props)P->Body->SetSimulatePhysics(false);if(Stage==0){Stage=1;Remaining=8;Learned=true;}else{Stage=3;Remaining=0;}}
void ALabGameMode::Tick(float Dt){Super::Tick(Dt);if(Playing()){Remaining=FMath::Max(0.f,Remaining-Dt);Current=0;for(auto& Pair:Chains)if(Active(Pair.Key))Current=FMath::Max(Current,Pair.Value.Members.Num());if(Remaining<=0)FinishRound();}else if(Stage==1){Remaining-=Dt;if(Remaining<=0)StartRound(true);}}
void ALabHUD::DrawHUD(){Super::DrawHUD();auto G=GM(GetWorld());if(!G||!Canvas)return;float W=Canvas->SizeX,H=Canvas->SizeY;float S=W/1280.f;auto Text=[&](const FString& T,float X,float Y,float Size,FLinearColor Color){DrawText(T,Color,X*S,Y*S,GEngine->GetLargeFont(),Size*S*1.5f);};
 bool Paused=UGameplayStatics::IsGamePaused(GetWorld());if(G->HideHUD&&!Paused)return;
 DrawRect(FLinearColor(.013,.015,.015,.9),28*S,24*S,1224*S,80*S);Text(TEXT("BAD CAT LAB"),48,40,1.1,Cream);Text(G->Stage==2?TEXT("02 / BOUNCE ROOM"):TEXT("01 / HOUSE RULES"),48,73,.5,Orange);Text(FString::Printf(TEXT("%02d SEC"),FMath::CeilToInt(G->Remaining)),560,45,1.2,Cream);Text(FString::Printf(TEXT("%05d"),G->Score),1080,42,1.3,Orange);
 DrawRect(FLinearColor(.90,.83,.72,.95),38*S,116*S,420*S,68*S);
 Text(FString::Printf(TEXT("POUNCE  %s %s %s"),G->Pounces>=1?TEXT("O"):TEXT("-"),G->Pounces>=2?TEXT("O"):TEXT("-"),G->Pounces>=3?TEXT("O"):TEXT("-")),50,124,.7,Ink);
 Text(G->Stage==2?(G->Challenge?TEXT("CHALLENGE COMPLETE +500"):TEXT("ONE POUNCE > FIVE OBJECTS")):TEXT("A CUP. AN ANGLE. A PERFECT ACCIDENT."),50,156,.65,Ink);
 if(G->Current>1)Text(FString::Printf(TEXT("CHAIN %02d  x%d"),G->Current,FMath::Min(G->Current,5)),510,130,1.1,Orange);
 DrawRect(FLinearColor(.013,.015,.015,.85),28*S,H-60*S,1224*S,38*S);Text(TEXT("WASD MOVE   MOUSE LOOK   CLICK SWIPE   HOLD POUNCE   SPACE JUMP   R RETRY   ESC PAUSE"),45,H/S-50,.5,Cream);
 if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){if(Cat->Charging){DrawRect(Ink,W/2-80*S,H*.7,160*S,8*S);DrawRect(Orange,W/2-80*S,H*.7,160*S*Cat->Charge/.8f,8*S);}if(Cat->Innocent)Text(TEXT("It was like this when I got here."),430,H/S-100,.75,Ink);}
 if(GetWorld()->GetTimeSeconds()<G->QuipUntil)Text(G->Quip,410,190,.8,Orange);
 if(G->Stage==1||G->Stage==3||Paused){DrawRect(FLinearColor(.013,.015,.015,.93),W*.18,H*.29,W*.64,H*.42);
 if(Paused){Text(TEXT("CAT NAP"),460,H/S*.35,2,Cream);Text(TEXT("ESC RESUME   /   R RETRY"),440,H/S*.52,.8,Orange);}
 else if(G->Stage==1){Text(TEXT("FUSION / FIXED RESULT"),385,H/S*.34,1.2,Orange);Text(TEXT("CAT + BOUNCE CAT"),395,H/S*.43,1.3,Cream);Text(TEXT("THE ROOM NOW BREAKS PHYSICS."),365,H/S*.55,.9,Cream);}
 else{Text(TEXT("DAMAGE REPORT"),420,H/S*.34,1.6,Orange);Text(FString::Printf(TEXT("BEST CHAIN %d    PAWS USED %d    SCORE %d"),G->Best,G->Swipes,G->Score),330,H/S*.46,.85,Cream);Text(TEXT("R BOUNCE AGAIN    /    T FULL EXPERIMENT"),355,H/S*.57,.8,Cream);}}
}
