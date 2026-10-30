#include "LabGame.h"
#include "LabCatalog.h"
#include "Engine/Texture2D.h"
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
 PropId=Id;Heavy=IsHeavy;Mutated=Bounce&&!Heavy;BaseScore=Id==15?300:(Id==11||Id==13?150:100);
 const TCHAR* Names[]={TEXT("SM_Cup"),TEXT("SM_Tray"),TEXT("SM_RoundedBlock"),TEXT("SM_RoundedBlock"),TEXT("SM_RoundedBlock"),TEXT("SM_Book"),nullptr,TEXT("SM_Book"),TEXT("SM_Book"),TEXT("SM_Book"),TEXT("SM_Cup"),nullptr,TEXT("SM_Car"),TEXT("SM_Lamp"),TEXT("SM_Book"),TEXT("SM_TV"),TEXT("SM_Book"),TEXT("SM_Cup")};auto Mesh=(Id>=0&&Id<18&&Names[Id])?ArtShape(Names[Id]):nullptr;Body->SetStaticMesh(Mesh?Mesh:Shape(Ball?TEXT("Sphere"):TEXT("Cube")));Body->SetWorldScale3D(Scale);Paint(Body,Color);
 Physics=NewObject<UPhysicalMaterial>(this);Physics->Friction=.45f;Physics->Restitution=Mutated?.93f:.18f;Physics->bOverrideRestitutionCombineMode=true;Physics->RestitutionCombineMode=EFrictionCombineMode::Max;Body->SetPhysMaterialOverride(Physics);
 Body->SetSimulatePhysics(true);Body->SetMassOverrideInKg(NAME_None,Heavy?18.f:1.5f);Body->SetLinearDamping(Mutated?.12f:.5f);Body->SetAngularDamping(.7f);
 Body->SetPhysicsMaxAngularVelocityInDegrees(Heavy?100.f:650.f);
 if(Heavy){Body->BodyInstance.bLockXTranslation=true;Body->BodyInstance.bLockYTranslation=true;Body->BodyInstance.bLockZRotation=true;Body->BodyInstance.bLockXRotation=true;Body->BodyInstance.SetDOFLock(EDOFMode::SixDOF);}
}
void ALabProp::Strike(int32 Chain,FVector V){auto G=GM(GetWorld());if(!G||!G->Playing())return;ChainId=Chain;G->Affect(this,Chain);Body->AddImpulse(V,NAME_None,true);G->TryResonate(this,Chain,V.GetSafeNormal());}
void ALabProp::Hit(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,FVector Impulse,const FHitResult& R){
 auto G=GM(GetWorld());if(!G||!G->Playing())return;
 auto P=Cast<ALabProp>(Other);
 if(P && Impulse.Size()>15){if(G->Active(ChainId)&&!G->Active(P->ChainId)){P->ChainId=ChainId;G->Affect(P,ChainId);}else if(!G->Active(ChainId)&&G->Active(P->ChainId)){ChainId=P->ChainId;G->Affect(this,ChainId);}}
 if(Mutated && G->Active(ChainId)&&Impulse.Size()>80 && Bounces<4){Bounces++;Body->AddImpulse(R.ImpactNormal*(G->RecipeId>=0?LabCatalog::Recipe(G->RecipeId).BounceImpulse:160),NAME_None,true);if(Bounces==4){Physics->Restitution=.15f;Body->SetPhysMaterialOverride(Physics);Body->SetLinearDamping(2);}}
}
void ALabProp::Tick(float Dt){Super::Tick(Dt);FVector V=Body->GetPhysicsLinearVelocity();if(V.Size()>1800)Body->SetPhysicsLinearVelocity(V.GetClampedToMaxSize(1800));if(GetActorLocation().Z< -150 || GetActorLocation().Size()>2500){Body->SetSimulatePhysics(false);SetActorHiddenInGame(true);SetActorEnableCollision(false);}}
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
void ALabCat::SetupPlayerInputComponent(UInputComponent* I){Super::SetupPlayerInputComponent(I);I->BindAxis("Forward",this,&ALabCat::Forward);I->BindAxis("Right",this,&ALabCat::Right);I->BindAxis("Turn",this,&ALabCat::Turn);I->BindAxis("Look",this,&ALabCat::Look);I->BindAction("Attack",IE_Pressed,this,&ALabCat::Press);I->BindAction("Attack",IE_Released,this,&ALabCat::Release);I->BindAction("Jump",IE_Pressed,this,&ALabCat::Hop);I->BindAction("Retry",IE_Pressed,this,&ALabCat::Retry).bExecuteWhenPaused=true;I->BindAction("Pause",IE_Pressed,this,&ALabCat::Pause).bExecuteWhenPaused=true;I->BindAction("HUD",IE_Pressed,this,&ALabCat::ToggleHUD);I->BindAction("Camera",IE_Pressed,this,&ALabCat::ToggleCamera);I->BindAction("Tutorial",IE_Pressed,this,&ALabCat::Tutorial).bExecuteWhenPaused=true;I->BindAction("One",IE_Pressed,this,&ALabCat::PickOne);I->BindAction("Two",IE_Pressed,this,&ALabCat::PickTwo);I->BindAction("Three",IE_Pressed,this,&ALabCat::PickThree);I->BindAction("Confirm",IE_Pressed,this,&ALabCat::ConfirmSelection);I->BindAction("Select",IE_Pressed,this,&ALabCat::OpenSelection).bExecuteWhenPaused=true;}
void ALabCat::Forward(float V){auto G=GM(GetWorld());if(G&&G->Playing()){AddMovementInput(FRotator(0,GetControlRotation().Yaw,0).Vector(),V);if(V!=0)Idle=0;}}
void ALabCat::Right(float V){auto G=GM(GetWorld());if(G&&G->Playing()){AddMovementInput(FRotationMatrix(FRotator(0,GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y),V);if(V!=0)Idle=0;}}
void ALabCat::Turn(float V){if(!FixedCamera&&GM(GetWorld())&&GM(GetWorld())->Playing())AddControllerYawInput(FMath::Clamp(V,-3.f,3.f));}
void ALabCat::Look(float V){if(!FixedCamera&&GM(GetWorld())&&GM(GetWorld())->Playing())AddControllerPitchInput(FMath::Clamp(V,-3.f,3.f));}
void ALabCat::Press(){auto G=GM(GetWorld());if(G&&G->Playing()&&!UGameplayStatics::IsGamePaused(GetWorld())){Charging=true;Charge=0;Idle=0;}}
void ALabCat::Release(){auto G=GM(GetWorld());if(!Charging||!G||!G->Playing())return;Charging=false;bool Big=Charge>=.25f&&G->Pounces>0;int32 Chain=G->NewChain(Big);G->Swipes++;if(Big){G->Pounces--;G->UsedPounces++;}FVector Dir=FRotator(0,GetControlRotation().Yaw,0).Vector();SetActorRotation(Dir.Rotation());ActionTime=.25f;Idle=0;
 float Power=Big?FMath::Lerp(650.f,1250.f,FMath::Clamp(Charge/.8f,0.f,1.f)):LabCatalog::Cat(G->SelectedCat).Swipe;
 FVector Origin=GetActorLocation()+FVector(0,0,30);TArray<FHitResult> Hits;FCollisionQueryParams Q;Q.AddIgnoredActor(this);
 GetWorld()->SweepMultiByObjectType(Hits,Origin,Origin+Dir*(Big?170:95),FQuat::Identity,FCollisionObjectQueryParams(ECC_PhysicsBody),FCollisionShape::MakeSphere(Big?35:22),Q);
 Hits.Sort([](const FHitResult& A,const FHitResult& B){return A.Distance<B.Distance;});
 TSet<ALabProp*> HitProps;for(auto& H:Hits){auto P=Cast<ALabProp>(H.GetActor());if(P&&!HitProps.Contains(P)){HitProps.Add(P);P->Strike(Chain,Dir*Power+FVector(0,0,Big?190:100));break;}}
 if(Big)LaunchCharacter(Dir*650+FVector(0,0,180),true,true);
}
void ALabCat::Hop(){if(auto G=GM(GetWorld());G&&G->Playing()){Jump();Idle=0;}}
void ALabCat::Retry(){UGameplayStatics::SetGamePaused(GetWorld(),false);if(auto G=GM(GetWorld()))G->Retry();}
void ALabCat::Tutorial(){UGameplayStatics::SetGamePaused(GetWorld(),false);if(auto G=GM(GetWorld()))G->Retry(true);}
void ALabCat::Pause(){auto G=GM(GetWorld());if(!G||G->Stage==4||G->Stage==5)return;Charging=false;Charge=0;UGameplayStatics::SetGamePaused(GetWorld(),!UGameplayStatics::IsGamePaused(GetWorld()));}
void ALabCat::ToggleHUD(){if(auto G=GM(GetWorld()))G->HideHUD=!G->HideHUD;}
void ALabCat::ToggleCamera(){FixedCamera=!FixedCamera;Arm->bUsePawnControlRotation=!FixedCamera;Arm->SetRelativeRotation(FRotator(-40,-30,0));Arm->TargetArmLength=FixedCamera?650:430;}
void ALabCat::ResetCat(){if(auto PC=Cast<APlayerController>(Controller)){PC->PlayerCameraManager->ViewPitchMin=-65;PC->PlayerCameraManager->ViewPitchMax=-10;PC->FlushPressedKeys();}SetActorLocation(FVector(-220,0,35),false,nullptr,ETeleportType::TeleportPhysics);GetCharacterMovement()->StopMovementImmediately();Charging=false;Charge=0;Idle=0;ActionTime=0;if(Controller)Controller->SetControlRotation(FRotator(-32,0,0));}
void ALabCat::SetMutation(bool On){auto G=GM(GetWorld());ApplyIdentity(G?G->SelectedCat:0,On);}
void ALabCat::ApplyIdentity(int32 Id,bool Fused){
 const auto& D=LabCatalog::Cat(Id);GetCharacterMovement()->MaxWalkSpeed=D.Speed;GetCharacterMovement()->JumpZVelocity=D.Jump;
 const FString MeshPath(D.Mesh);const FString Folder=FPaths::GetPath(MeshPath);
 const FString RigPath=MeshPath.Replace(TEXT("SM_"),TEXT("SK_"));auto Rig=LoadObject<USkeletalMesh>(nullptr,*RigPath);
 GetMesh()->SetSkeletalMesh(Rig);GetMesh()->SetVisibility(Rig!=nullptr);GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);Locomotion.Empty();ActiveAnimation=INDEX_NONE;
 if(Rig){const FBox B=Rig->GetImportedBounds().GetBox();const float Scale=64.f/FMath::Max(1.f,B.GetSize().Z);GetMesh()->SetRelativeScale3D(FVector(Scale));GetMesh()->SetRelativeLocation(FVector(0,0,-30-B.Min.Z*Scale));GetMesh()->SetRelativeRotation(FRotator(0,-90,0));for(const TCHAR* A:{TEXT("A_Idle"),TEXT("A_Run"),TEXT("A_Walk")})Locomotion.Add(LoadObject<UAnimSequence>(nullptr,*(Folder/FString(A))));}
 UStaticMesh* M=Rig?nullptr:LoadObject<UStaticMesh>(nullptr,D.Mesh);HeroMesh->EmptyOverrideMaterials();HeroMesh->SetStaticMesh(M);HeroMesh->SetVisibility(M!=nullptr);Visual->SetVisibility(M==nullptr&&Rig==nullptr,true);
 if(M){const FBox B=M->GetBoundingBox();const float Scale=64.f/FMath::Max(1.f,B.GetSize().Z);HeroMesh->SetRelativeScale3D(FVector(Scale));HeroMesh->SetRelativeLocation(FVector(0,0,-30-B.Min.Z*Scale));HeroMesh->SetRelativeRotation(FRotator(0,-90,0));}
 TArray<UStaticMeshComponent*> Parts;GetComponents(Parts);for(auto C:Parts){if(C==HeroMesh||FusionParts.Contains(C))continue;FString N=C->GetName();Paint(C,N==TEXT("Nose")||N.StartsWith(TEXT("Eye"))?Ink:N==TEXT("Patch")||N==TEXT("Tail")?Id==1?FLinearColor(.6,.12,.025):Ink:D.Color);}
 auto G=GM(GetWorld());const FLinearColor Accent=G&&G->RecipeId>=0?LabCatalog::Recipe(G->RecipeId).Color:Orange;
 for(auto Part:FusionParts)if(IsValid(Part))Part->DestroyComponent();FusionParts.Empty();
 if(Fused&&G&&G->RecipeId>=0){const auto& R=LabCatalog::Recipe(G->RecipeId);auto AddAccessory=[&](const TCHAR* Name,FVector Pos,FVector Scale){auto M=ArtShape(Name);if(!M)return;auto P=NewObject<UStaticMeshComponent>(this);P->SetStaticMesh(M);P->SetupAttachment(RootComponent);P->RegisterComponent();P->SetCollisionEnabled(ECollisionEnabled::NoCollision);P->SetRelativeLocation(Pos);P->SetRelativeScale3D(Scale);FusionParts.Add(P);};if(R.Bounce)AddAccessory(TEXT("SM_Spring"),FVector(-20,0,22),FVector(.13,.13,.30));if(R.Resonance){AddAccessory(TEXT("SM_Speaker"),FVector(-5,-17,8),FVector(.13,.10,.20));AddAccessory(TEXT("SM_Speaker"),FVector(-5,17,8),FVector(.13,.10,.20));}for(auto P:Paws)Paint(P,Accent);}
}
void ALabCat::PickOne(){if(auto G=GM(GetWorld()))G->SelectCat(0);}
void ALabCat::PickTwo(){if(auto G=GM(GetWorld()))G->SelectCat(1);}
void ALabCat::PickThree(){if(auto G=GM(GetWorld()))G->SelectCat(2);}
void ALabCat::ConfirmSelection(){if(auto G=GM(GetWorld()))G->ConfirmSelection();}
void ALabCat::OpenSelection(){UGameplayStatics::SetGamePaused(GetWorld(),false);if(auto G=GM(GetWorld()))G->OpenSelection();}
void ALabCat::Tick(float Dt){Super::Tick(Dt);static bool Logged=false;if(!Logged&&FParse::Param(FCommandLine::Get(),TEXT("CatArtReview"))&&GetWorld()->GetTimeSeconds()>2){Logged=true;UE_LOG(LogTemp,Display,TEXT("CAT_REVIEW actor=%s camera=%s cameraRotation=%s mesh=%s scale=%s bounds=%s visible=%d"),*GetActorLocation().ToString(),*Camera->GetComponentLocation().ToString(),*Camera->GetComponentRotation().ToString(),*GetMesh()->GetComponentLocation().ToString(),*GetMesh()->GetComponentScale().ToString(),*GetMesh()->Bounds.GetBox().ToString(),GetMesh()->IsVisible());}auto G=GM(GetWorld());if(!G)return;Idle+=Dt;ActionTime=FMath::Max(0.f,ActionTime-Dt);if(Charging){Charge=FMath::Min(.8f,Charge+Dt);FVector A=GetActorLocation();DrawDebugDirectionalArrow(GetWorld(),A,A+FRotator(0,GetControlRotation().Yaw,0).Vector()*(80+Charge*120),24,FColor(255,106,43),false,0,0,3);}
 Innocent=Idle>1.2f&&G->Best>=4&&G->Current==0;float Speed=GetVelocity().Size2D();float T=GetWorld()->GetTimeSeconds();Visual->SetRelativeRotation(FRotator(ActionTime>0?-18:Innocent?12:0,0,0));Visual->SetRelativeLocation(FVector(0,0,-23+(Speed>20?FMath::Sin(T*20)*2:0)));
 for(int i=0;i<Paws.Num();i++)Paws[i]->SetRelativeRotation(FRotator(Speed>20?FMath::Sin(T*20+i*PI)*28:0,0,0));Tail->SetRelativeRotation(FRotator(-30,FMath::Sin(T*3)*15,0));if(HeroMesh->GetStaticMesh()){HeroMesh->SetRelativeRotation(FRotator(ActionTime>0?-12:Innocent?8:0,-90,FMath::Sin(T*12)*FMath::Clamp(Speed/300.f,0.f,1.f)*3));}if(GetMesh()->GetSkeletalMeshAsset()){int32 Desired=Speed>180?1:Speed>15?2:0;if(Desired!=ActiveAnimation&&Locomotion.IsValidIndex(Desired)&&Locomotion[Desired]){GetMesh()->PlayAnimation(Locomotion[Desired],true);ActiveAnimation=Desired;}GetMesh()->SetRelativeRotation(FRotator(ActionTime>0?-12:Innocent?8:0,-90,0));}if(GetActorLocation().Z< -100)ResetCat();
}
ALabGameMode::ALabGameMode(){PrimaryActorTick.bCanEverTick=true;DefaultPawnClass=ALabCat::StaticClass();HUDClass=ALabHUD::StaticClass();}
void ALabGameMode::BeginPlay(){Super::BeginPlay();BuildRoom();OpenSelection();
#if !UE_BUILD_SHIPPING
 if(FParse::Param(FCommandLine::Get(),TEXT("CatArtReview"))){int32 ReviewCat=0,ReviewRecipe=-1;FParse::Value(FCommandLine::Get(),TEXT("ReviewCat="),ReviewCat);FParse::Value(FCommandLine::Get(),TEXT("ReviewRecipe="),ReviewRecipe);SelectedCat=FMath::Clamp(ReviewCat,0,2);RecipeId=FMath::Clamp(ReviewRecipe,-1,2);StartRound(RecipeId>=0);if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->FixedCamera=true;Cat->Arm->bUsePawnControlRotation=false;Cat->Arm->SetRelativeRotation(FRotator(-25,-25,0));Cat->Arm->TargetArmLength=260;Cat->SetActorRotation(FRotator(0,155,0));}}
#endif
}
void ALabGameMode::SetMenuInput(bool Enabled){if(auto PC=GetWorld()->GetFirstPlayerController()){PC->bShowMouseCursor=Enabled;PC->bEnableClickEvents=Enabled;if(Enabled){FInputModeGameAndUI Mode;Mode.SetHideCursorDuringCapture(false);PC->SetInputMode(Mode);}else PC->SetInputMode(FInputModeGameOnly());}}
bool ALabGameMode::BounceRule() const{return Stage==2&&RecipeId>=0&&LabCatalog::Recipe(RecipeId).Bounce;}
bool ALabGameMode::ResonanceRule() const{return Stage==2&&RecipeId>=0&&LabCatalog::Recipe(RecipeId).Resonance;}
void ALabGameMode::OpenSelection(){for(auto A:PresentationCats)if(IsValid(A))A->Destroy();PresentationCats.Empty();NextChain=1;QuipUntil=0;for(auto P:Props)if(IsValid(P))P->Destroy();Props.Empty();Chains.Empty();Score=Best=Current=Swipes=UsedPounces=0;Remaining=45;Pounces=3;Stage=4;RecipeId=INDEX_NONE;Learned=false;Challenge=false;Quip.Empty();HideHUD=false;SetMenuInput(true);if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->ResetCat();Cat->ApplyIdentity(SelectedCat,false);}}
void ALabGameMode::SelectCat(int32 Index){if(Index<0||Index>=LabCatalog::CatCount)return;if(Stage==4){SelectedCat=Index;PartnerCat=(Index+1)%LabCatalog::CatCount;if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0)))Cat->ApplyIdentity(Index,false);}else if(Stage==5&&Index!=SelectedCat){PartnerCat=Index;}}
void ALabGameMode::ConfirmSelection(){if(Stage==4){Score=Best=Swipes=UsedPounces=0;StartRound(false);}else if(Stage==5){const int32 R=LabCatalog::RecipeFor(SelectedCat,PartnerCat);if(R==INDEX_NONE)return;RecipeId=R;Stage=1;Remaining=4;SetMenuInput(false);UpdatePresentation();}else if(Stage==3)Retry();}
void ALabGameMode::BeginFusionChoice(){Stage=5;Remaining=0;PartnerCat=(SelectedCat+1)%LabCatalog::CatCount;SetMenuInput(true);if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->Charging=false;Cat->GetCharacterMovement()->StopMovementImmediately();}}
void ALabGameMode::UpdatePresentation(){for(auto A:PresentationCats)if(IsValid(A))A->Destroy();PresentationCats.Empty();if(Stage!=1)return;auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));if(!Cat)return;for(int i=0;i<2;i++){const auto& D=LabCatalog::Cat(i?PartnerCat:SelectedCat);auto M=LoadObject<UStaticMesh>(nullptr,D.Mesh);if(!M)continue;auto A=GetWorld()->SpawnActor<AStaticMeshActor>();auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);C->SetStaticMesh(M);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetWorldScale3D(FVector(55.f/FMath::Max(1.f,M->GetBoundingBox().GetSize().Z)));PresentationCats.Add(A);}}
void ALabGameMode::TryResonate(ALabProp* Source,int32 Id,FVector Direction){if(!ResonanceRule()||!Source)return;auto Chain=Chains.Find(Id);if(!Chain||Chain->Resonated||!Active(Id))return;Chain->Resonated=true;const auto& R=LabCatalog::Recipe(RecipeId);TArray<ALabProp*> Candidates;for(auto P:Props){if(IsValid(P)&&P!=Source&&!P->Heavy&&!Chain->Members.Contains(P->PropId)&&FVector::DistSquared(P->GetActorLocation(),Source->GetActorLocation())<FMath::Square(145.f))Candidates.Add(P);}Candidates.Sort([&](const ALabProp& A,const ALabProp& B){return FVector::DistSquared(A.GetActorLocation(),Source->GetActorLocation())<FVector::DistSquared(B.GetActorLocation(),Source->GetActorLocation());});for(int i=0;i<FMath::Min(R.PulseTargets,Candidates.Num());i++){auto P=Candidates[i];P->ChainId=Id;Affect(P,Id);FVector D=(P->GetActorLocation()-Source->GetActorLocation()).GetSafeNormal();P->Body->AddImpulse((D*.65f+Direction*.35f).GetSafeNormal()*380+FVector(0,0,100),NAME_None,true);}Quip=TEXT("共振扩散！附近的物件一起遭殃。");QuipUntil=GetWorld()->GetTimeSeconds()+2;}
static void Furniture(UWorld* W,FVector Pos,FVector Scale,FLinearColor Color,const TCHAR* Mesh=TEXT("Cube")){
 auto A=W->SpawnActor<AStaticMeshActor>(Pos,FRotator::ZeroRotator);auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);auto Rounded=FString(Mesh)==TEXT("Cube")&&Scale.Z>.15f?ArtShape(TEXT("SM_RoundedBlock")):nullptr;C->SetStaticMesh(Rounded?Rounded:Shape(Mesh));C->SetWorldScale3D(Scale);C->SetCollisionProfileName(TEXT("BlockAll"));Paint(C,Color);
}
void ALabGameMode::BuildRoom(){
 for(int i=0;i<2;i++){auto Fill=GetWorld()->SpawnActor<APointLight>(FVector(-140,i?200:-200,230),FRotator::ZeroRotator);auto Light=Cast<UPointLightComponent>(Fill->GetLightComponent());Light->SetIntensity(60000);Light->SetAttenuationRadius(1400);Light->SetLightColor(FLinearColor(1,.94,.86));Light->SetCastShadows(false);}
 Furniture(GetWorld(),FVector(0,0,-12),FVector(9,7,.24),Cream);Furniture(GetWorld(),FVector(440,0,150),FVector(.2,7,3),Cream);Furniture(GetWorld(),FVector(0,-350,150),FVector(9,.2,3),Blue);Furniture(GetWorld(),FVector(0,350,150),FVector(9,.2,3),Cream);
 Furniture(GetWorld(),FVector(10,0,35),FVector(1.8,1.25,.18),Blue);for(int i=0;i<4;i++)Furniture(GetWorld(),FVector(i<2?-60:80,i%2?45:-45,13),FVector(.12,.12,.26),Ink);
 Furniture(GetWorld(),FVector(-60,-245,30),FVector(2.6,.8,.6),Blue);Furniture(GetWorld(),FVector(-60,-285,65),FVector(2.6,.2,1),Blue);
 Furniture(GetWorld(),FVector(260,-220,40),FVector(1.9,.75,.15),Cream);Furniture(GetWorld(),FVector(260,235,40),FVector(2.3,.8,.2),Blue);
 auto L=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,300),FRotator(-55,-35,0));L->GetLightComponent()->SetIntensity(4);L->GetLightComponent()->SetLightColor(FLinearColor(1,.92,.8));
 auto FillSun=GetWorld()->SpawnActor<ADirectionalLight>(FVector(0,0,300),FRotator(-35,145,0));FillSun->GetLightComponent()->SetIntensity(1.5f);FillSun->GetLightComponent()->SetCastShadows(false);FillSun->GetLightComponent()->SetLightColor(FLinearColor(.85,.92,1));
 auto S=GetWorld()->SpawnActor<ASkyLight>();S->GetLightComponent()->SetMobility(EComponentMobility::Movable);S->GetLightComponent()->SetIntensity(1.2);S->GetLightComponent()->RecaptureSky();
}
void ALabGameMode::SpawnProps(bool Bounce){
 const FVector Positions[]={ {-55,0,57},{-10,0,52},{35,0,57},{35,0,82},{35,0,107},{-40,43,51},{130,-220,65},{190,-220,68},{220,-220,68},{250,-220,68},{290,-220,63},{330,-220,65},{165,235,61},{215,235,80},{265,235,68},{280,235,105},{330,235,64},{360,235,64}};
 const FVector Scales[]={ {.22,.22,.28},{.45,.42,.12},{.25,.25,.25},{.25,.25,.25},{.25,.25,.25},{.3,.12,.08},{.3,.3,.3},{.15,.35,.42},{.15,.35,.42},{.15,.35,.42},{.28,.28,.3},{.3,.3,.35},{.42,.3,.22},{.18,.18,.65},{.18,.42,.38},{.12,.85,.65},{.12,.25,.3},{.22,.22,.28}};
 for(int i=0;i<18;i++){auto P=GetWorld()->SpawnActor<ALabProp>(Positions[i],FRotator::ZeroRotator);P->Configure(i,Scales[i],i%3==0?Orange:i%3==1?Cream:Blue,i==6||i==11,i==15,Bounce);Props.Add(P);}
}
void ALabGameMode::StartRound(bool Bounce){for(auto P:Props)if(IsValid(P))P->Destroy();Props.Empty();Chains.Empty();Current=0;NextChain=1;Stage=Bounce?2:0;Remaining=Bounce?60:45;Pounces=3;Challenge=false;for(auto A:PresentationCats)if(IsValid(A))A->Destroy();PresentationCats.Empty();SpawnProps(BounceRule());SetMenuInput(false);if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){Cat->ResetCat();Cat->SetMutation(Bounce);}}
void ALabGameMode::Retry(bool Full){Score=Best=Swipes=UsedPounces=0;Quip.Empty();if(Stage==4)return;if(Full){Learned=false;RecipeId=INDEX_NONE;}if(Stage==5||RecipeId==INDEX_NONE)StartRound(false);else StartRound(Learned);}
int32 ALabGameMode::NewChain(bool Big){FLabChain C;C.Id=NextChain++;C.Pounce=Big;C.Last=GetWorld()->GetTimeSeconds();Chains.Add(C.Id,C);return C.Id;}
bool ALabGameMode::Active(int32 Id) const {const auto C=Chains.Find(Id);return C&&GetWorld()->GetTimeSeconds()-C->Last<=2.5;}
void ALabGameMode::Affect(ALabProp* P,int32 Id){auto C=Chains.Find(Id);if(!Playing()||!C||!Active(Id)||C->Members.Contains(P->PropId))return;C->Members.Add(P->PropId);C->Last=GetWorld()->GetTimeSeconds();int32 N=C->Members.Num();Current=N;Best=FMath::Max(Best,N);if(!P->Scored){P->Scored=true;Score+=P->BaseScore*FMath::Min(N,5);}if(Stage==2&&C->Pounce&&N>=5&&!Challenge){Challenge=true;Score+=500;Quip=TEXT("一扑五件！这爪有点东西。");QuipUntil=C->Last+3;}if(P->PropId==15){Paint(P->Body,Orange);Quip=TEXT("信号已被猫接管。");QuipUntil=C->Last+3;}}
void ALabGameMode::FinishRound(){Score+=Pounces*150;Chains.Empty();Current=0;for(auto P:Props)P->Body->SetSimulatePhysics(false);if(Stage==0){Learned=true;BeginFusionChoice();}else{Stage=3;Remaining=0;SetMenuInput(true);}}
void ALabGameMode::Tick(float Dt){Super::Tick(Dt);if(Playing()){Remaining=FMath::Max(0.f,Remaining-(FParse::Param(FCommandLine::Get(),TEXT("CatArtReview"))?0:Dt));Current=0;for(auto& Pair:Chains)if(Active(Pair.Key))Current=FMath::Max(Current,Pair.Value.Members.Num());if(Remaining<=0)FinishRound();}else if(Stage==1){Remaining-=Dt;auto Cat=UGameplayStatics::GetPlayerPawn(GetWorld(),0);if(Cat){for(int i=0;i<PresentationCats.Num();i++){float Side=i?1.f:-1.f;auto A=PresentationCats[i];A->SetActorLocation(Cat->GetActorLocation()+FVector(100,Side*FMath::Max(0.f,Remaining-1)*28,40+FMath::Sin(Remaining*3)*12));A->SetActorRotation(FRotator(0,Remaining*90+Side*60,0));}}if(Remaining<=0)StartRound(true);}}
void ALabHUD::NotifyHitBoxClick(FName Name){Super::NotifyHitBoxClick(Name);auto G=GM(GetWorld());if(!G)return;const FString Key=Name.ToString();if(Key.StartsWith(TEXT("Cat")))G->SelectCat(FCString::Atoi(*Key.Right(1)));else if(Name==TEXT("Confirm"))G->ConfirmSelection();else if(Name==TEXT("Retry"))G->Retry();else if(Name==TEXT("Select"))G->OpenSelection();}
void ALabHUD::DrawHUD(){
 Super::DrawHUD();auto G=GM(GetWorld());if(!G||!Canvas)return;
 const float W=Canvas->SizeX,H=Canvas->SizeY,S=FMath::Min(W/1280.f,H/720.f),OX=(W-1280*S)/2,OY=(H-720*S)/2;
 if(!ChineseFont){ChineseFont=NewObject<UFont>(this);ChineseFont->FontCacheType=EFontCacheType::Runtime;ChineseFont->LegacyFontSize=18;ChineseFont->GetMutableInternalCompositeFont()=FCompositeFont(FName("Regular"),FPaths::ProjectContentDir()/TEXT("UI/Fonts/DroidSansFallback.ttf"),EFontHinting::Default,EFontLoadingPolicy::LazyLoad);}
 if(Portraits.Num()==0){for(int i=0;i<3;i++)Portraits.Add(LoadObject<UTexture2D>(nullptr,LabCatalog::Cat(i).Portrait));}
 auto Text=[&](const FString& T,float X,float Y,float Size,FLinearColor Color){DrawText(T,Color,OX+X*S,OY+Y*S,ChineseFont,Size*S);};
 auto Box=[&](float X,float Y,float BW,float BH,FLinearColor Color){DrawRect(Color,OX+X*S,OY+Y*S,BW*S,BH*S);};
 auto Pic=[&](int32 I,float X,float Y,float Size){if(Portraits.IsValidIndex(I)&&Portraits[I])DrawTexture(Portraits[I],OX+X*S,OY+Y*S,Size*S,Size*S,0,0,1,1,FLinearColor::White);};
 auto Button=[&](FName Id,const FString& Label,float X,float Y,float BW,bool Primary){Box(X,Y,BW,48,Primary?Orange:Ink);Text(Label,X+20,Y+11,.82,Cream);AddHitBox(FVector2D(OX+X*S,OY+Y*S),FVector2D(BW*S,48*S),Id,true);};
 const FLinearColor Paper(.95,.92,.86),Muted(.30,.32,.31),Card(.99,.98,.95),Line(.80,.78,.72);
 const bool Paused=UGameplayStatics::IsGamePaused(GetWorld());
 if(G->Stage==4||G->Stage==5){
  DrawRect(Paper,0,0,W,H);const bool Fusion=G->Stage==5;
  Text(TEXT("合成大坏猫"),52,34,1.65,Ink);Text(Fusion?TEXT("实验 02 / 选择合成搭档"):TEXT("实验 01 / 领走你的麻烦"),820,47,.66,Muted);
  Box(52,100,1176,2,Ink);Text(Fusion?TEXT("两只坏猫，会合成什么？"):TEXT("今天，谁来拆家？"),52,120,1.25,Ink);
  Text(Fusion?TEXT("选择另一只猫，预览配方后开始合成。保留主猫外观，获得新的能力。"):TEXT("三只猫，三种手感。先制造事故，再合成改写规则。"),52,164,.67,Muted);
  for(int i=0;i<3;i++){
   const float X=52+i*400;const auto& D=LabCatalog::Cat(i);const bool Locked=Fusion&&i==G->SelectedCat;const bool Selected=i==(Fusion?G->PartnerCat:G->SelectedCat);
   Box(X,213,376,327,Selected?Ink:Line);Box(X+2,215,372,323,Card);Pic(i,X+69,220,235);
   if(Selected)Box(X+2,215,372,5,Orange);
   Text(FString::Printf(TEXT("0%d  %s"),i+1,D.Name),X+22,456,1.1,Ink);Text(D.Breed,X+22,494,.61,Muted);
   Text(Locked?TEXT("主猫"):Selected?TEXT("已选"):FString::Printf(TEXT("按 %d 选择"),i+1),X+270,466,.57,Locked?Muted:Orange);
   if(!Locked)AddHitBox(FVector2D(OX+X*S,OY+213*S),FVector2D(376*S,327*S),FName(*FString::Printf(TEXT("Cat%d"),i)),true);
  }
  if(Fusion){const int R=LabCatalog::RecipeFor(G->SelectedCat,G->PartnerCat);if(R>=0){const auto& Recipe=LabCatalog::Recipe(R);Text(FString::Printf(TEXT("%s + %s → %s"),LabCatalog::Cat(G->SelectedCat).Name,LabCatalog::Cat(G->PartnerCat).Name,Recipe.Name),52,567,1.0,Ink);Text(Recipe.Description,52,605,.64,Muted);}}
  else {const auto& D=LabCatalog::Cat(G->SelectedCat);Text(D.Motto,52,568,1.0,Ink);Text(G->SelectedCat==0?TEXT("挥爪精准 · 移动均衡"):G->SelectedCat==1?TEXT("跳得更高 · 挥爪更有力"):TEXT("移动更快 · 适合寻找连锁起点"),52,607,.65,Muted);}
  Button(TEXT("Confirm"),Fusion?TEXT("Enter  开始合成"):TEXT("Enter  开始拆家"),944,582,284,true);
  Box(52,660,1176,1,Line);Text(TEXT("WASD 移动    左键轻点挥爪 / 长按扑击    空格跳跃    Tab 换猫"),52,679,.52,Muted);return;
 }
 if(G->HideHUD&&!Paused)return;
 Box(26,22,300,69,FLinearColor(.02,.025,.025,.88));Text(TEXT("合成大坏猫"),42,32,.95,Cream);
 const FString Identity=G->Stage==2&&G->RecipeId>=0?LabCatalog::Recipe(G->RecipeId).Name:LabCatalog::Cat(G->SelectedCat).Name;
 Text(Identity,42,64,.52,Orange);
 Box(1070,22,184,69,FLinearColor(.02,.025,.025,.88));Text(FString::Printf(TEXT("%02d"),FMath::CeilToInt(G->Remaining)),1090,28,1.65,Cream);Text(TEXT("秒"),1192,53,.6,Cream);
 Box(26,105,300,111,FLinearColor(.95,.92,.86,.96));Text(FString::Printf(TEXT("%05d"),G->Score),42,113,1.25,Ink);Text(TEXT("拆家得分"),193,130,.5,Muted);
 Text(FString::Printf(TEXT("剩余扑击  %s  %s  %s"),G->Pounces>=1?TEXT("●"):TEXT("○"),G->Pounces>=2?TEXT("●"):TEXT("○"),G->Pounces>=3?TEXT("●"):TEXT("○")),42,165,.7,Ink);
 if(G->Current>1){Text(FString::Printf(TEXT("%02d 连锁 ×%d"),G->Current,FMath::Min(G->Current,5)),510,44,1.1,Orange);}
 Box(26,641,1228,53,FLinearColor(.02,.025,.025,.85));
 Text(G->Stage==2?(G->Challenge?TEXT("挑战达成：一扑五件！"):TEXT("挑战：一次扑击碰到 5 件物品")):TEXT("找好角度，一爪制造事故。"),42,651,.67,Cream);
 Text(TEXT("左键 挥爪 / 蓄力扑击    R 重试    Tab 换猫    Esc 暂停"),630,657,.5,Cream);
 if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){if(Cat->Charging){Box(540,545,200,7,Ink);Box(540,545,200*Cat->Charge/.8f,7,Orange);}if(Cat->Innocent){Box(423,580,430,39,Paper);Text(TEXT("我来的时候就这样了。"),465,587,.78,Ink);}}
 if(GetWorld()->GetTimeSeconds()<G->QuipUntil){Box(352,110,576,40,Paper);Text(G->Quip,374,120,.67,Ink);}
 if(G->Stage==1){
  Box(0,0,1280,720,FLinearColor(.02,.025,.025,.88));const auto& R=LabCatalog::Recipe(G->RecipeId);float T=FMath::Clamp(1-G->Remaining/4,0.f,1.f);
  Text(TEXT("合成实验 / 配方已确认"),442,80,1.0,R.Color);Pic(G->SelectedCat,285+T*100,170,240*(1-T*.25f));Pic(G->PartnerCat,755-T*100,170,240*(1-T*.25f));
  Text(TEXT("＋"),615,257,2,R.Color);Text(R.Name,477,450,2.1,Cream);Text(R.Description,350,535,.82,Cream);Box(340,611,600,4,Muted);Box(340,611,600*T,4,R.Color);return;
 }
 if(G->Stage==3&&!Paused){
  Box(245,153,790,418,FLinearColor(.95,.92,.86,.98));Text(TEXT("今日拆家账单"),292,185,1.4,Ink);Text(Identity,292,236,.67,Muted);Pic(G->SelectedCat,755,225,225);
  Text(FString::Printf(TEXT("%05d"),G->Score),292,285,2.5,Orange);Text(FString::Printf(TEXT("最高连锁 %d   /   出爪 %d 次"),G->Best,G->Swipes),292,369,.87,Ink);
  Text(TEXT("猫方声明：我来的时候就这样了。"),292,425,.65,Muted);Button(TEXT("Retry"),TEXT("R  再拆一次"),292,489,300,true);Button(TEXT("Select"),TEXT("Tab  换猫合成"),616,489,330,false);
 }
 if(Paused){Box(295,220,690,278,FLinearColor(.02,.025,.025,.96));Text(TEXT("坏猫打个盹"),470,270,1.8,Cream);Text(TEXT("Esc 继续拆家    R 重试    Tab 换猫"),425,390,.8,Orange);}
}
