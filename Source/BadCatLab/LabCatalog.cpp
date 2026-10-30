#include "LabCatalog.h"
const FLabCatDefinition& LabCatalog::Cat(int32 Index)
{
 static const FLabCatDefinition Cats[]={
  {TEXT("阿坏"),TEXT("奶牛猫 / 精准派"),TEXT("我只是碰了一下。"),TEXT("/Game/UI/Cats/T_Ahuai"),TEXT("/Game/Characters/Ahuai/SM_Ahuai"),FLinearColor(.90,.83,.72),300,390,430},
  {TEXT("橘团"),TEXT("橘猫 / 弹跳派"),TEXT("落地？下次一定。"),TEXT("/Game/UI/Cats/T_Jutuan"),TEXT("/Game/Characters/Jutuan/SM_Jutuan"),FLinearColor(1,.32,.07),275,455,460},
  {TEXT("乌梅"),TEXT("黑猫 / 共振派"),TEXT("这屋需要一点低音。"),TEXT("/Game/UI/Cats/T_Wumei"),TEXT("/Game/Characters/Wumei/SM_Wumei"),FLinearColor(.045,.055,.075),330,400,390}
 };
 return Cats[FMath::Clamp(Index,0,CatCount-1)];
}
int32 LabCatalog::RecipeFor(int32 A,int32 B)
{
 if(A<0||B<0||A>=CatCount||B>=CatCount||A==B)return INDEX_NONE;
 const int32 Mask=(1<<A)|(1<<B);
 return Mask==3?0:Mask==5?1:2;
}
const FLabRecipe& LabCatalog::Recipe(int32 Index)
{
 static const FLabRecipe Recipes[]={
  {TEXT("弹力惯犯"),TEXT("小物件获得强弹性，借墙接出下一击。"),TEXT("瞄准墙角，让杯子替你拐弯。"),FLinearColor(1,.25,.06),true,false,160,0},
  {TEXT("共振惯犯"),TEXT("每条连锁的首击向附近三件物品传递冲量。"),TEXT("先挑密集处下爪，再让连锁散开。"),FLinearColor(.12,.55,.68),false,true,0,3},
  {TEXT("蹦迪惯犯"),TEXT("较轻的弹跳搭配双目标共振，制造交叉连锁。"),TEXT("用家具反弹，把两条路线接起来。"),FLinearColor(.64,.23,.50),true,true,90,2}
 };
 return Recipes[FMath::Clamp(Index,0,2)];
}
