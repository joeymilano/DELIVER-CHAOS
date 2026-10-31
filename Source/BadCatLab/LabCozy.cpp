#include "LabGame.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"

namespace {
UStaticMesh* CozyMesh(const TCHAR* Name){return LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/CozyLocal/%s"),Name),nullptr,LOAD_Quiet|LOAD_NoWarn);}
void SetCozy(UStaticMeshComponent* C,UStaticMesh* M,float Height){C->SetStaticMesh(M);C->EmptyOverrideMaterials();C->SetWorldScale3D(FVector(Height/FMath::Max(1.f,M->GetBoundingBox().GetSize().Z)));}
}
void ALabGameMode::BuildCozyCorner(){
 auto Piece=[&](const TCHAR* Name,FVector P,float H,float Yaw=0){if(auto M=CozyMesh(Name)){auto A=GetWorld()->SpawnActor<AStaticMeshActor>(P,FRotator(0,Yaw,0));auto C=A->GetStaticMeshComponent();C->SetMobility(EComponentMobility::Movable);SetCozy(C,M,H);C->SetCollisionProfileName(TEXT("BlockAll"));A->Tags.Add(TEXT("CozyDecor"));}};
 // North wall tea party; keep centre aisle and existing high catwalk clear.
 Piece(TEXT("Pumpkin_Fireplace"),FVector(-40,635,0),150,180);
 Piece(TEXT("Candy_Hutch"),FVector(270,625,0),170,180);
 Piece(TEXT("Candy_Buffet"),FVector(-300,560,0),85,180);
 Piece(TEXT("Broom_and_Hat_Stand"),FVector(-390,635,0),140);
 Piece(TEXT("Ghost_Window_Garland"),FVector(-30,675,240),35,180);
 Piece(TEXT("Pumpkin_Fairy_Lights"),FVector(100,670,290),30,180);
 Piece(TEXT("Moon_Phase_Board"),FVector(-300,675,200),35,180);
}
void ALabGameMode::SpawnCozyProps(){
 auto Prop=[&](int Id,const TCHAR* Model,const TCHAR* Label,FVector P,float H,int Price){if(auto M=CozyMesh(Model)){
  auto A=GetWorld()->SpawnActor<ALabProp>(P,FRotator::ZeroRotator);A->Configure(Id,FVector(.3),FLinearColor::White,false,false,false);A->Body->SetSimulatePhysics(false);SetCozy(A->Body,M,H);A->Body->SetSimulatePhysics(true);A->Body->SetMassOverrideInKg(NAME_None,1.2f);A->InitialPosition=P;A->ItemName=Label;A->BaseScore=Price;Props.Add(A);
 }};
 Prop(400,TEXT("Crystal_Ball"),TEXT("占卜水晶球"),FVector(-350,560,87),25,699);
 Prop(401,TEXT("Plum_Teapot"),TEXT("紫梅茶壶"),FVector(-270,550,87),22,259);
 Prop(402,TEXT("Moon_Cookies"),TEXT("月亮饼干"),FVector(-300,585,87),7,49);
 Prop(403,TEXT("Candy_Jar_Set"),TEXT("糖果罐"),FVector(230,625,172),28,159);
 Prop(404,TEXT("Ghost_Cookie_Tray"),TEXT("幽灵饼干"),FVector(300,625,172),8,89);
 Prop(405,TEXT("Pumpkin_Tea_Cake"),TEXT("南瓜茶点"),FVector(-10,615,152),22,129);
 Prop(406,TEXT("Little_Happy_Ghost"),TEXT("开心幽灵"),FVector(80,600,0),38,99);
 Prop(407,TEXT("Friendly_Cat_Plush"),TEXT("黑猫玩偶"),FVector(-130,575,0),35,169);
 Prop(408,TEXT("Potion_Trio"),TEXT("魔药瓶"),FVector(-370,590,87),24,299);
}
