#include "LabGame.h"
#include "LabCatalog.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "CanvasItem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"
void ALabHUD::NotifyHitBoxClick(FName Name){Super::NotifyHitBoxClick(Name);auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());if(!G)return;FString S=Name.ToString();if(S.StartsWith(TEXT("Cat")))G->SelectCat(FCString::Atoi(*S.Right(1)));else if(Name==TEXT("Confirm"))G->ConfirmSelection();else if(Name==TEXT("Retry"))G->Retry();else if(Name==TEXT("Select"))G->Lab();else if(Name==TEXT("Practice"))G->SetPractice();else if(Name==TEXT("Solo")){G->RecipeId=INDEX_NONE;G->StartRound(false);}}
void ALabHUD::DrawHUD(){
 Super::DrawHUD();auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());if(!G||!Canvas)return;
 const float W=Canvas->SizeX,H=Canvas->SizeY,S=FMath::Min(W/1280.f,H/720.f),OX=(W-1280*S)/2,OY=(H-720*S)/2;
 const FLinearColor Paper(.95,.92,.86),Ink(.025,.028,.03),Orange(1,.145,.025),Muted(.3,.32,.31),Line(.8,.78,.72),Card(.99,.98,.95);
 if(!ChineseFont){ChineseFont=NewObject<UFont>(this);ChineseFont->FontCacheType=EFontCacheType::Runtime;ChineseFont->LegacyFontSize=18;ChineseFont->GetMutableInternalCompositeFont()=FCompositeFont(FName("Regular"),FPaths::ProjectContentDir()/TEXT("UI/Fonts/DroidSansFallback.ttf"),EFontHinting::Default,EFontLoadingPolicy::LazyLoad);}
 if(Portraits.IsEmpty())for(int I=0;I<4;I++)Portraits.Add(LoadObject<UTexture2D>(nullptr,LabCatalog::Cat(I).Portrait));
 auto Text=[&](const FString& T,float X,float Y,float Size,FLinearColor C){DrawText(T,C,OX+X*S,OY+Y*S,ChineseFont,Size*S);};
 auto Box=[&](float X,float Y,float A,float B,FLinearColor C){DrawRect(C,OX+X*S,OY+Y*S,A*S,B*S);};
 auto Pic=[&](int I,float X,float Y,float Size){if(Portraits.IsValidIndex(I)&&Portraits[I])DrawTexture(Portraits[I],OX+X*S,OY+Y*S,Size*S,Size*S,0,0,1,1);};
 auto Silhouette=[&](float X,float Y,float Size){
  // A real silhouette drawn without exposing the locked portrait's background.
  Box(X+Size*.24f,Y+Size*.34f,Size*.52f,Size*.52f,Ink);Box(X+Size*.18f,Y+Size*.18f,Size*.64f,Size*.40f,Ink);
  auto Tri=[&](FVector2D A,FVector2D B,FVector2D C){FCanvasTriangleItem Item(FVector2D(OX,OY)+A*S,FVector2D(OX,OY)+B*S,FVector2D(OX,OY)+C*S,GWhiteTexture);Item.SetColor(Ink);Canvas->DrawItem(Item);};
  Tri(FVector2D(X+Size*.18f,Y+Size*.25f),FVector2D(X+Size*.18f,Y),FVector2D(X+Size*.42f,Y+Size*.22f));Tri(FVector2D(X+Size*.58f,Y+Size*.22f),FVector2D(X+Size*.82f,Y),FVector2D(X+Size*.82f,Y+Size*.25f));
  Text(TEXT("？"),X+Size*.40f,Y+Size*.32f,Size/110,Paper);
 };
 auto Button=[&](FName Id,const FString& T,float X,float Y,float BW,bool Main){Box(X,Y,BW,44,Main?Orange:Ink);Text(T,X+14,Y+10,.7,Paper);AddHitBox(FVector2D(OX+X*S,OY+Y*S),FVector2D(BW*S,44*S),Id,true);};
 const bool Paused=UGameplayStatics::IsGamePaused(GetWorld());
 if(G->Stage==4||G->Stage==5){
  DrawRect(Paper,0,0,W,H);const bool Lab=G->Stage==5;
  Text(TEXT("DELIVER CHAOS / 合成大坏猫"),44,28,1.15,Ink);Text(TEXT("万圣节派对 · 猫方概不负责"),860,40,.6,Muted);Box(44,80,1192,2,Ink);
  Text(Lab?TEXT("挑一只搭档，把两条本能合成在一起。"):TEXT("领走你的麻烦。拆开藏身处，发现更多猫。"),44,100,.9,Ink);
  for(int I=0;I<4;I++){
   const float X=44+I*302;bool Unlocked=G->Unlocked(I),Selected=I==(Lab?G->PartnerCat:G->SelectedCat),Main=Lab&&I==G->SelectedCat;
   Box(X,148,286,272,Selected?Orange:Line);Box(X+2,150,282,268,Card);
   if(Unlocked)Pic(I,X+56,158,174);else Silhouette(X+68,165,150);
   Text(Unlocked?FString::Printf(TEXT("%d  %s"),I+1,LabCatalog::Cat(I).Name):TEXT("未发现"),X+15,337,1,Ink);
   Text(Unlocked?LabCatalog::Cat(I).Breed:I==1?TEXT("线索：快递纸箱"):I==2?TEXT("线索：书架上的杯子"):TEXT("线索：最高的南瓜灯"),X+15,375,.58,Muted);
   if(Main)Text(TEXT("主猫"),X+215,342,.5,Orange);else if(Selected)Text(TEXT("已选"),X+215,342,.5,Orange);
   if(Unlocked&&!Main)AddHitBox(FVector2D(OX+X*S,OY+148*S),FVector2D(286*S,272*S),FName(*FString::Printf(TEXT("Cat%d"),I)),true);
  }
  if(Lab){int R=LabCatalog::RecipeFor(G->SelectedCat,G->PartnerCat);if(R>=0){bool Ready=G->RecipeUnlocked(R);Text(Ready?LabCatalog::Recipe(R).Name:TEXT("隐藏配方 / 条件未满足"),44,447,1.15,Ready?Orange:Ink);Text(Ready?LabCatalog::Recipe(R).Description:LabCatalog::Recipe(R).Hint,44,487,.7,Muted);if(Ready)Button(TEXT("Confirm"),TEXT("Enter  合成出门"),940,456,296,true);}
   Button(TEXT("Select"),TEXT("重新选主猫"),44,561,230,false);Button(TEXT("Solo"),TEXT("先单猫出门"),294,561,230,false);
  }else{
   Text(LabCatalog::Cat(G->SelectedCat).Motto,44,444,.8,Ink);
   Button(TEXT("Confirm"),TEXT("Enter  出发 / 选搭档"),922,450,314,true);
   Button(TEXT("Practice"),G->Practice?TEXT("F 切换：自由拆家 · 不计榜"):TEXT("F 切换：60 秒主人到家"),44,491,400,false);
   Text(TEXT("本机图鉴"),44,556,.7,Muted);Text(FString::Printf(TEXT("猫 %d / 4    配方 %d / 6"),G->Progress?FMath::CountBits((uint32)G->Progress->UnlockedMask):1,G->Progress?FMath::CountBits((uint32)G->Progress->RecipesMask):0),44,590,.65,Ink);
  }
  const bool BigKnown=G->Progress&&(G->Progress->RecipesMask&(1<<5));
  if(BigKnown)Pic(0,580,445,160);else Silhouette(590,445,150);
  Text(BigKnown?TEXT("大坏猫 / 已发现"):TEXT("？？？ / 巨型隐藏猫"),550,604,.7,Orange);
  Text(TEXT("收齐 4 猫 + 完成 3 配方"),562,635,.5,Muted);
  Box(44,666,1192,1,Line);Text(TEXT("1–4 选择    E 慢推    Shift 暴冲    Q 音波    长按左键扭屁股，松开扑击"),44,683,.55,Muted);return;
 }
 if(G->HideHUD&&!Paused)return;
 Box(24,20,360,105,FLinearColor(.02,.025,.025,.90));Text(TEXT("本次拆家损失"),40,29,.6,Paper);Text(FString::Printf(TEXT("￥%s"),*FText::AsNumber(G->Score).ToString()),40,53,1.65,Orange);
 const FString Identity=G->RecipeId>=0?LabCatalog::Recipe(G->RecipeId).Name:LabCatalog::Cat(G->SelectedCat).Name;
 Text(Identity,40,100,.53,Paper);
 Box(1040,20,216,105,Ink);Text(G->Practice?TEXT("随便拆"):FString::Printf(TEXT("%02d 秒"),FMath::CeilToInt(G->Remaining)),1056,36,1.3,Paper);Text(G->Practice?TEXT("自由模式 · 不计排行榜"):TEXT("距离主人回家"),1056,88,.5,Paper);
 Box(24,139,360,64,Paper);Text(FString::Printf(TEXT("超级扑  %s  %s  %s"),G->Pounces>0?TEXT("●"):TEXT("○"),G->Pounces>1?TEXT("●"):TEXT("○"),G->Pounces>2?TEXT("●"):TEXT("○")),40,150,.72,Ink);Text(TEXT("普通扑不限 · 蓄满 1 秒升级超级扑"),40,179,.45,Muted);
 Box(910,145,346,122,G->CameraBroken?Ink:FLinearColor(.22,.025,.025,.94));Text(G->CameraBroken?TEXT("监控：无信号"):G->CameraFlash>0?TEXT("● 被拍到了！"):TEXT("● 宠物摄像头在线"),928,159,.7,Paper);
 Text(FString::Printf(TEXT("嫌疑 %d / 5   乖猫认证 %d"),G->Suspicion,G->GoodCat),928,194,.55,Paper);Text(G->CameraBroken?TEXT("完美犯罪 / 继续拆"):TEXT("红色视野内停下装无辜"),928,229,.5,Orange);
 if(!G->CameraCaption.IsEmpty()){Box(325,564,630,37,Paper);Text(G->CameraCaption,340,574,.52,Ink);}
 if(GetWorld()->GetTimeSeconds()<G->QuipUntil){Box(300,126,680,42,Paper);Text(G->Quip,315,138,.58,Ink);}
 if(GetWorld()->GetTimeSeconds()<G->PopUntil){FVector Screen=Project(G->PopLocation);if(Screen.Z>0)DrawText(G->PopText,Orange,Screen.X,Screen.Y-30,ChineseFont,.75*S);}
 if(auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))){
  if(Cat->Charging){Box(490,505,300,8,Ink);Box(490,505,300*Cat->Charge/1.25f,8,Orange);Text(Cat->Charge>=1.05&&G->Pounces>0?TEXT("超级扑就绪！"):TEXT("扭扭扭……松开扑！"),515,525,.65,Orange);}
  if(Cat->Innocent)Text(TEXT("我来的时候就这样了。"),500,525,.7,Paper);
  if(auto P=Cat->FindTarget(200,.1f);P&&G->Playing())Text(FString::Printf(TEXT("%s  ￥%d  /  E 慢推"),*P->ItemName,P->BaseScore),450,610,.65,Orange);
 }
 Box(24,658,1232,44,Ink);Text(TEXT("左键 挥爪 / 长按扑击    E 慢推    Shift 暴冲    Q 音波    Space 跳跃"),40,670,.53,Paper);Text(TEXT("R 重试   Tab 图鉴   V 震动   Esc 暂停"),827,670,.48,Paper);
 if(G->Stage==1){Box(0,0,1280,720,FLinearColor(.02,.025,.025,.93));const auto& R=LabCatalog::Recipe(G->RecipeId);float T=FMath::Clamp(1-G->Remaining/4,0.f,1.f);Text(TEXT("两条猫的本能，合成一场事故。"),400,95,1,Paper);Pic(G->SelectedCat,280+T*100,180,220);Pic(G->PartnerCat,780-T*100,180,220);Text(TEXT("＋"),615,260,1.8,Orange);Text(R.Name,470,463,1.9,Orange);Text(R.Description,390,545,.8,Paper);Box(340,615,600*T,5,Orange);return;}
 if(G->Stage==3&&!Paused){
  Box(175,120,930,500,Paper);Text(TEXT("主人回家了。账单来了。"),210,145,1.25,Ink);Text(Identity,210,193,.6,Muted);Text(FString::Printf(TEXT("损失 ￥%s"),*FText::AsNumber(G->Score).ToString()),210,233,1.8,Orange);
  Text(G->PerfectCrime?TEXT("猫方鉴定：完美犯罪"):G->Suspicion>=3?TEXT("猫方鉴定：当场抓包"):TEXT("猫方鉴定：证据不足"),210,300,.85,Ink);
  Text(FString::Printf(TEXT("乖猫认证 %d   /   出爪 %d 次   /   已收集 %d 只猫"),G->GoodCat,G->Swipes,G->Progress?FMath::CountBits((uint32)G->Progress->UnlockedMask):1),210,343,.6,Muted);
  Text(TEXT("本机拆家榜"),760,155,.9,Ink);
  if(G->Progress)for(int I=0;I<G->Progress->Board.Num();I++){auto& Row=G->Progress->Board[I];Text(FString::Printf(TEXT("%d  ￥%s  %s"),I+1,*FText::AsNumber(Row.Money).ToString(),*Row.Cat),760,208+I*39,.54,I==0?Orange:Muted);}
  Text(G->Practice?TEXT("自由模式不写入榜单"):TEXT("榜单与图鉴保存在本机"),760,425,.5,Muted);
  Text(TEXT("下一次，换两只猫再来？"),210,420,.8,Ink);
  Button(TEXT("Select"),TEXT("Enter  回合成实验室"),210,515,390,true);Button(TEXT("Retry"),TEXT("R  原配方再拆一次"),630,515,435,false);
 }
 if(Paused){Box(295,220,690,278,Ink);Text(TEXT("坏猫打个盹"),470,270,1.8,Paper);Text(TEXT("Esc 继续    R 重试    Tab 图鉴"),420,390,.8,Orange);}
}
