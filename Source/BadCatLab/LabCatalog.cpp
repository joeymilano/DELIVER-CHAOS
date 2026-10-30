#include "LabCatalog.h"
const FLabCatDefinition& LabCatalog::Cat(int32 I){
 static const FLabCatDefinition C[]={
 {TEXT("阿坏"),TEXT("奶牛 / 疯"),TEXT("暴冲撞上东西，会弹开。"),TEXT("/Game/UI/Cats/T_Ahuai"),TEXT("/Game/Characters/Ahuai/SM_Ahuai"),FLinearColor(.9,.83,.72),340,440,480,CatTrait::Wild},
 {TEXT("橘团"),TEXT("橘猫 / 重"),TEXT("跳得越高，落地越有分量。"),TEXT("/Game/UI/Cats/T_Jutuan"),TEXT("/Game/Characters/Jutuan/SM_Jutuan"),FLinearColor(1,.32,.07),310,560,520,CatTrait::Heavy},
 {TEXT("乌梅"),TEXT("黑猫 / 吵"),TEXT("按 Q 喵一声，前方一起遭殃。"),TEXT("/Game/UI/Cats/T_Wumei"),TEXT("/Game/Characters/Wumei/SM_Wumei"),FLinearColor(.045,.055,.075),350,440,460,CatTrait::Loud},
 {TEXT("大福"),TEXT("缅因 / 大"),TEXT("体型 1.8 倍，连电视也能推。"),TEXT("/Game/UI/Cats/T_Ahuai"),TEXT("/Game/Characters/Ahuai/SM_Ahuai"),FLinearColor(.5,.43,.36),300,440,850,CatTrait::Giant}
 };return C[FMath::Clamp(I,0,CatCount-1)];
}
int32 LabCatalog::RecipeFor(int32 A,int32 B){if(A<0||B<0||A>=CatCount||B>=CatCount||A==B)return INDEX_NONE;const int M=(1<<A)|(1<<B);return M==3?0:M==5?1:M==6?2:M==10?3:M==12?4:5;}
const FLabRecipe& LabCatalog::Recipe(int32 I){
 static const FLabRecipe R[]={
 {TEXT("弹跳炸弹"),TEXT("暴冲反弹 + 高处落地震荡"),TEXT("先爬高，再把派对坐塌。"),FLinearColor(1,.25,.06),false,false,0,0,CatTrait::Wild|CatTrait::Heavy,false},
 {TEXT("噪音炸弹"),TEXT("暴冲撞击时，自动放出一声喵"),TEXT("每次撞击都能引发扇形冲击。"),FLinearColor(.12,.55,.68),false,true,0,3,CatTrait::Wild|CatTrait::Loud,false},
 {TEXT("低音炮"),TEXT("落地震荡 + 前方音波"),TEXT("跳进密集物件中，再按 Q。"),FLinearColor(.64,.23,.50),false,true,0,3,CatTrait::Heavy|CatTrait::Loud,false},
 {TEXT("猫猫山"),TEXT("巨大体型 + 重量震荡"),TEXT("电视与南瓜，今天都得躺下。"),FLinearColor(1,.45,.1),false,false,0,0,CatTrait::Heavy|CatTrait::Giant,false},
 {TEXT("巨型扩音器"),TEXT("巨大体型 + 大范围音波"),TEXT("先完成任意两种普通配方。"),FLinearColor(.3,.65,.5),false,true,0,3,CatTrait::Loud|CatTrait::Giant,true},
 {TEXT("大坏猫"),TEXT("巨大体型 + 停不下来的暴冲"),TEXT("收齐四只猫，并完成三种其他配方。"),FLinearColor(1,.18,.03),false,false,0,0,CatTrait::Wild|CatTrait::Giant,true}
 };return R[FMath::Clamp(I,0,RecipeCount-1)];
}
