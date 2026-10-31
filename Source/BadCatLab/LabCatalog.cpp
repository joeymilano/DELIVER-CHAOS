#include "LabCatalog.h"
const FLabCatDefinition& LabCatalog::Cat(int32 I){
 static const FLabCatDefinition C[]={
 {TEXT("阿坏"),TEXT("奶牛猫 · 暴冲反弹"),TEXT("按 Shift 暴冲，撞上东西后弹开。"),TEXT("/Game/UI/Cats/T_Ahuai"),TEXT("/Game/Characters/Ahuai/SM_Ahuai"),FLinearColor(.9,.83,.72),340,440,480,CatTrait::Wild},
 {TEXT("橘团"),TEXT("橘猫 · 落地震荡"),TEXT("跳到高处再落下，震飞身边的东西。"),TEXT("/Game/UI/Cats/T_Jutuan"),TEXT("/Game/Characters/Jutuan/SM_Jutuan"),FLinearColor(1,.32,.07),310,560,520,CatTrait::Heavy},
 {TEXT("乌梅"),TEXT("黑猫 · 喵声冲击"),TEXT("按 Q 喵一声，把前方的东西震开。"),TEXT("/Game/UI/Cats/T_Wumei"),TEXT("/Game/Characters/Wumei/SM_Wumei"),FLinearColor(.045,.055,.075),350,440,460,CatTrait::Loud},
 {TEXT("大福"),TEXT("缅因猫 · 巨大体型"),TEXT("长到 1.8 倍大，连重家具也推得动。"),TEXT("/Game/UI/Cats/T_Dafu"),TEXT("/Game/Characters/Dafu/SM_Dafu"),FLinearColor(.5,.43,.36),300,440,850,CatTrait::Giant}
 };return C[FMath::Clamp(I,0,CatCount-1)];
}
int32 LabCatalog::RecipeFor(int32 A,int32 B){if(A<0||B<0||A>=CatCount||B>=CatCount||A==B)return INDEX_NONE;const int M=(1<<A)|(1<<B);return M==3?0:M==5?1:M==6?2:M==10?3:M==12?4:5;}
const FLabRecipe& LabCatalog::Recipe(int32 I){
 static const FLabRecipe R[]={
 {TEXT("弹跳炸弹"),TEXT("暴冲碰撞反弹，落地触发震荡。"),TEXT("从高处落地触发震荡。"),FLinearColor(1,.25,.06),false,false,0,0,CatTrait::Wild|CatTrait::Heavy,false},
 {TEXT("噪音炸弹"),TEXT("暴冲碰撞触发音波。"),TEXT("每次撞击都能引发扇形冲击。"),FLinearColor(.12,.55,.68),false,true,0,3,CatTrait::Wild|CatTrait::Loud,false},
 {TEXT("低音炮"),TEXT("落地触发震荡，按 Q 释放音波。"),TEXT("跳进密集物件中，再按 Q。"),FLinearColor(.64,.23,.50),false,true,0,3,CatTrait::Heavy|CatTrait::Loud,false},
 {TEXT("猫猫山"),TEXT("体型增大，落地震荡范围扩大。"),TEXT("从高处落地可推动重物。"),FLinearColor(1,.45,.1),false,false,0,0,CatTrait::Heavy|CatTrait::Giant,false},
 {TEXT("巨型扩音器"),TEXT("体型增大，音波范围扩大。"),TEXT("先完成任意两种普通配方。"),FLinearColor(.3,.65,.5),false,true,0,3,CatTrait::Loud|CatTrait::Giant,true},
 {TEXT("大坏猫"),TEXT("体型 1.8 倍，暴冲可推动重物。"),TEXT("找到全部 4 只猫，再合成 3 种不同的猫。"),FLinearColor(1,.18,.03),false,false,0,0,CatTrait::Wild|CatTrait::Giant,true}
 };return R[FMath::Clamp(I,0,RecipeCount-1)];
}
