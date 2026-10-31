#include "LabGame.h"
#include "LabCatalog.h"
#include "LabResident.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "CanvasItem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/Engine.h"

// All positions use a 1280 x 720 safe area. Typography uses measured glyph height,
// so the same hierarchy survives windowed, Retina and wider display sizes.
namespace {
FLinearColor Hex(const TCHAR* Value) { return FLinearColor::FromSRGBColor(FColor::FromHex(Value)); }
const FLinearColor Paper=Hex(TEXT("F3EBDD")), White=Hex(TEXT("FFFCF6")), Ink=Hex(TEXT("222323"));
const FLinearColor Orange=Hex(TEXT("FF6A2B")), Muted=Hex(TEXT("62645E")), Line=Hex(TEXT("DAD2C5"));
const FLinearColor Blue=Hex(TEXT("C9DCDF")), Green=Hex(TEXT("B8D4A6")), Red=Hex(TEXT("D83D32"));
const TCHAR* TraitName(int I) { static const TCHAR* T[]={TEXT("暴冲反弹"),TEXT("落地震荡"),TEXT("喵声冲击"),TEXT("巨大体型")};return T[FMath::Clamp(I,0,3)]; }
const TCHAR* Clue(int I) { static const TCHAR* T[]={TEXT("已经跟你回家了"),TEXT("拍开地上的快递纸箱"),TEXT("推倒书架上的杯子"),TEXT("打翻最高处的南瓜灯")};return T[FMath::Clamp(I,0,3)]; }
FString Money(int32 Value) { return TEXT("¥")+FText::AsNumber(Value).ToString(); }
}

void ALabHUD::NotifyHitBoxClick(FName Name) {
 Super::NotifyHitBoxClick(Name);
 auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());if(!G)return;
 auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
 if(Name==TEXT("TitleStart")){G->StartFromTitle();return;}
 if(Name==TEXT("TitleFree")){G->StartFromTitle(true);return;}
 if(Name==TEXT("TitleMusic")){G->MusicMuted=!G->MusicMuted;G->UpdateMusic();return;}
 if(Name==TEXT("Meme")){G->ToggleMemeGif();return;}
 if(Name==TEXT("Resume")){if(Cat)Cat->Pause();return;}
 if(Name==TEXT("Shake")){if(Cat)Cat->ToggleShake();return;}
 if(Name==TEXT("Retry")){UGameplayStatics::SetGamePaused(GetWorld(),false);G->Retry();return;}
 if(Name==TEXT("Select")){UGameplayStatics::SetGamePaused(GetWorld(),false);G->Lab();return;}
 const FString S=Name.ToString();
 if(S.StartsWith(TEXT("Cat")))G->SelectCat(FCString::Atoi(*S.Right(1)));
 else if(Name==TEXT("Confirm"))G->ConfirmSelection();
 else if(Name==TEXT("Practice"))G->SetPractice();
 else if(Name==TEXT("Solo")&&G->Stage==5){G->RecipeId=INDEX_NONE;G->StartRound(false);}
}

void ALabHUD::DrawHUD() {
 Super::DrawHUD();
 auto G=Cast<ALabGameMode>(GetWorld()->GetAuthGameMode());if(!G||!Canvas)return;
 auto Cat=Cast<ALabCat>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
 auto PC=GetWorld()->GetFirstPlayerController();
 const bool Paused=UGameplayStatics::IsGamePaused(GetWorld());
 const float W=Canvas->SizeX,H=Canvas->SizeY,S=FMath::Min(W/1280.f,H/720.f),OX=(W-1280*S)*.5f,OY=(H-720*S)*.5f;
 const double Now=GetWorld()->GetTimeSeconds();
 if(LastStage!=G->Stage){LastStage=G->Stage;StageEnteredAt=Now;LastCaption=G->CameraCaption;CaptionVisibleUntil=0;}
 if(LastCaption!=G->CameraCaption){LastCaption=G->CameraCaption;CaptionVisibleUntil=Now+4.5;}
 if(!ChineseFont){
  auto MakeFont=[&](const TCHAR* File){auto F=NewObject<UFont>(this);F->FontCacheType=EFontCacheType::Runtime;F->LegacyFontSize=64;F->GetMutableInternalCompositeFont()=FCompositeFont(FName("Regular"),FPaths::ProjectContentDir()/TEXT("UI/Fonts")/File,EFontHinting::Default,EFontLoadingPolicy::LazyLoad);return F;};
  ChineseFont=MakeFont(TEXT("NotoSansCJKsc-Regular.otf"));ChineseBoldFont=MakeFont(TEXT("NotoSansCJKsc-Bold.otf"));
 }
 if(Portraits.IsEmpty())for(int I=0;I<4;I++)Portraits.Add(LoadObject<UTexture2D>(nullptr,LabCatalog::Cat(I).Portrait));
 float BaseW=0,BaseH=0;Canvas->StrLen(ChineseFont,TEXT("国Ag"),BaseW,BaseH);BaseH=FMath::Max(1.f,BaseH)/1.22f;
 auto Width=[&](const FString& T,float Px,bool Bold=false){float A=0,B=0;Canvas->StrLen(Bold?ChineseBoldFont:ChineseFont,T,A,B);return A*Px/BaseH;};
 auto Text=[&](const FString& T,float X,float Y,float Px,FLinearColor Color,bool Bold=false){
  FCanvasTextItem Item(FVector2D(OX+X*S,OY+Y*S),FText::FromString(T),Bold?ChineseBoldFont:ChineseFont,Color);
  Item.Scale=FVector2D(Px*S/BaseH);Canvas->DrawItem(Item);
 };
 auto Fit=[&](const FString& T,float X,float Y,float Px,float MaxW,FLinearColor Color,bool Bold=false){Text(T,X,Y,FMath::Min(Px,Px*MaxW/FMath::Max(1.f,Width(T,Px,Bold))),Color,Bold);};
 auto Center=[&](const FString& T,float X,float Y,float Px,FLinearColor Color,bool Bold=false){Text(T,X-Width(T,Px,Bold)*.5f,Y,Px,Color,Bold);};
 auto Wrap=[&](const FString& T,float X,float Y,float Px,float MaxW,FLinearColor Color){
  FString Row;float DY=0;for(TCHAR C:T){FString Next=Row;Next.AppendChar(C);if(C=='\n'||Width(Next,Px)>MaxW){Text(Row,X,Y+DY,Px,Color);DY+=Px*1.6f;Row.Empty();if(C=='\n')continue;}Row.AppendChar(C);}if(!Row.IsEmpty())Text(Row,X,Y+DY,Px,Color);
 };
 auto Box=[&](float X,float Y,float BW,float BH,FLinearColor C){DrawRect(C,OX+X*S,OY+Y*S,BW*S,BH*S);};
 auto Tri=[&](FVector2D A,FVector2D B,FVector2D C,FLinearColor Color){FCanvasTriangleItem Item(FVector2D(OX,OY)+A*S,FVector2D(OX,OY)+B*S,FVector2D(OX,OY)+C*S,GWhiteTexture);Item.SetColor(Color);Canvas->DrawItem(Item);};
 auto Ellipse=[&](float X,float Y,float RX,float RY,FLinearColor C){for(int I=0;I<32;I++){float A=2*PI*I/32,B=2*PI*(I+1)/32;Tri(FVector2D(X,Y),FVector2D(X+FMath::Cos(A)*RX,Y+FMath::Sin(A)*RY),FVector2D(X+FMath::Cos(B)*RX,Y+FMath::Sin(B)*RY),C);}};
 auto Round=[&](float X,float Y,float BW,float BH,float R,FLinearColor C){
  R=FMath::Min(R,FMath::Min(BW,BH)*.5f);Box(X+R,Y,BW-2*R,BH,C);Box(X,Y+R,R,BH-2*R,C);Box(X+BW-R,Y+R,R,BH-2*R,C);
  for(int Corner=0;Corner<4;Corner++){float CX=X+(Corner==0||Corner==3?R:BW-R),CY=Y+(Corner<2?R:BH-R);for(int I=0;I<8;I++){float A=(180+Corner*90+I*90.f/8)*PI/180,B=A+PI/16;Tri(FVector2D(CX,CY),FVector2D(CX+FMath::Cos(A)*R,CY+FMath::Sin(A)*R),FVector2D(CX+FMath::Cos(B)*R,CY+FMath::Sin(B)*R),C);}}
 };
 auto Pic=[&](int I,float X,float Y,float Size){if(Portraits.IsValidIndex(I)&&Portraits[I])DrawTexture(Portraits[I],OX+X*S,OY+Y*S,Size*S,Size*S,0,0,1,1);};
 auto CatMark=[&](float X,float Y,float R,FLinearColor C){
  Ellipse(X,Y+R*.12f,R,R*.72f,C);Tri(FVector2D(X-R*.93f,Y),FVector2D(X-R*.90f,Y-R),FVector2D(X-R*.22f,Y-R*.40f),C);Tri(FVector2D(X+R*.22f,Y-R*.40f),FVector2D(X+R*.90f,Y-R),FVector2D(X+R*.93f,Y),C);
 };
 auto Key=[&](const FString& T,float X,float Y,float BW,bool Dark=false){Round(X,Y,BW,26,5,Dark?Hex(TEXT("41423E")):Line);Center(T,X+BW/2,Y+5,14,Dark?Paper:Ink);};
 float MX=-999,MY=-999;if(PC&&PC->GetMousePosition(MX,MY)){MX=(MX-OX)/S;MY=(MY-OY)/S;}
 auto Hover=[&](float X,float Y,float BW,float BH){return MX>=X&&MX<X+BW&&MY>=Y&&MY<Y+BH;};
 auto Hit=[&](FName Id,float X,float Y,float BW,float BH){AddHitBox(FVector2D(OX+X*S,OY+Y*S),FVector2D(BW*S,BH*S),Id,true);};
 auto Button=[&](FName Id,const FString& Label,const FString& Shortcut,float X,float Y,float BW,bool Primary,bool Enabled=true){
  const bool Focused=Enabled&&G->FocusedMenuAction()==Id;
  bool Hot=Enabled&&Hover(X,Y,BW,52);if(Focused)Round(X-4,Y-4,BW+8,60,15,Orange);FLinearColor BG=Enabled?(Primary?(Hot?Hex(TEXT("FF844D")):Orange):(Hot?Hex(TEXT("41423E")):Hex(TEXT("333530")))):Line;
  Round(X,Y,BW,52,12,BG);Fit(Label,X+18,Y+15,20,BW-(Shortcut.IsEmpty()?36:102),Enabled?(Primary?Ink:Paper):Muted,true);
  if(!Shortcut.IsEmpty())Key(Shortcut,X+BW-70,Y+13,54,!Primary&&Enabled);
  if(Enabled)Hit(Id,X,Y,BW,52);
 };
 auto Pill=[&](const FString& T,float X,float Y,FLinearColor BG,FLinearColor FG){float BW=Width(T,14)+24;Round(X,Y,BW,28,14,BG);Text(T,X+12,Y+6,14,FG);};
 auto Background=[&](){DrawRect(Paper,0,0,W,H);Box(0,0,1280,8,Orange);};
 auto Brand=[&](){Text(TEXT("Delivery Chaos"),44,28,13,Muted,true);};
 auto Footer=[&](const FString& T){Box(44,690,1192,1,Line);Text(T,44,701,12,Muted);};


 auto Coin=[&](FVector2D P,float Size,float Spin,float Alpha){
  auto Tint=[&](const TCHAR* V){auto C=Hex(V);C.A=Alpha;return C;};
  auto Oval=[&](FVector2D At,float RX,float RY,FLinearColor C){for(int I=0;I<16;I++){float A=2*PI*I/16,B=2*PI*(I+1)/16;Tri(At,At+FVector2D(FMath::Cos(A)*RX,FMath::Sin(A)*RY),At+FVector2D(FMath::Cos(B)*RX,FMath::Sin(B)*RY),C);}};
  const float RX=(2.2f+FMath::Abs(FMath::Cos(Spin))*5.8f)*Size,RY=9*Size;
  Oval(P+FVector2D(0,2*Size),RX+1.5f*Size,RY+1.5f*Size,Tint(TEXT("895016")));
  Oval(P,RX,RY,Tint(TEXT("FFBF32")));
  Oval(P+FVector2D(-Size,-Size),FMath::Max(Size,RX-2*Size),RY-2*Size,Tint(TEXT("FFE688")));
  if(RX>4*Size)Box(P.X-Size*.7f,P.Y-4*Size,Size*1.4f,8*Size,Tint(TEXT("CE861C")));
 };
 auto MoneyFeedback=[&](){
  const double RealNow=FPlatformTime::Seconds();TArray<FBox2D> Labels;
  for(const auto& B:G->MoneyBursts){
   const float Age=RealNow-B.Born;if(Age<0||Age>1.5f)continue;
   const FVector Projected=Project(B.Origin);if(Projected.Z<=0)continue;
   FVector2D Origin((Projected.X-OX)/S,(Projected.Y-OY)/S);Origin.X=FMath::Clamp(Origin.X,40.f,1240.f);Origin.Y=FMath::Clamp(Origin.Y,185.f,610.f);
   const float Tier=B.Amount>=1000?1.3f:1.f;
   if(Age<.3f){float T=Age/.3f;auto C=Hex(TEXT("FFD161"));C.A=(1-T)*.7f;
    for(int I=0;I<5;I++){float A=I*2*PI/5;FVector2D D(FMath::Cos(A),FMath::Sin(A)),N(-D.Y,D.X),P=Origin+D*(18+T*30)*Tier;Tri(P-D*5*Tier,P+N*2*Tier,P+D*5*Tier,C);Tri(P-D*5*Tier,P-N*2*Tier,P+D*5*Tier,C);}
   }
   const int Count=B.CoinCount();
   for(int I=0;I<Count;I++){
    const float Start=.48f+I*.025f,Travel=.55f,T=FMath::Clamp((Age-Start)/Travel,0.f,1.f);if(T>=1)continue;
    const float A=-PI*.9f+(PI*.8f*I/FMath::Max(1,Count-1));const FVector2D V=FVector2D(FMath::Cos(A),FMath::Sin(A))*(180+(B.Seed+I*17)%65);
    auto BurstAt=[&](float Time){return Origin+V*Time+FVector2D(0,115*Time*Time);};
    FVector2D P=BurstAt(FMath::Min(Age,Start));float Size=Tier;
    if(Age>=Start){const FVector2D Target(150,80),Control((P.X+Target.X)*.5f,FMath::Min(P.Y,Target.Y)-100);P=(1-T)*(1-T)*P+2*(1-T)*T*Control+T*T*Target;Size*=1-T*.4f;}
    Coin(P,Size,Age*14+I*1.7f,1-T*.35f);
   }
   if(Age<1.35f){
    const float Fade=1-FMath::Clamp((Age-.9f)/.45f,0.f,1.f),Pop=Age<.16f?FMath::Lerp(.65f,1.18f,FMath::Sin(Age/.16f*HALF_PI)):FMath::Lerp(1.18f,1.f,FMath::Clamp((Age-.16f)/.16f,0.f,1.f));
    const float Px=(B.Amount>=1000?32:25)*Pop;const FString Label=TEXT("+")+Money(B.Amount);const float LabelW=Width(Label,Px,true);
    float X=FMath::Clamp(Origin.X-LabelW*.5f+(B.Seed%3-1)*14*Age,24.f,1256.f-LabelW),Y=FMath::Max(180.f,Origin.Y-30-Age*55);
    // Separate simultaneous amounts without moving the physical burst origin.
    for(int Attempt=0;Attempt<12;Attempt++){
     FBox2D Bounds(FVector2D(X-5,Y-4),FVector2D(X+LabelW+5,Y+Px*1.35f));bool Overlap=false;
     for(const auto& Other:Labels)if(Bounds.Intersect(Other)){Overlap=true;break;}
     if(!Overlap)break;
     if(Y-Px*1.5f>=180)Y-=Px*1.5f;else X=FMath::Clamp(X+LabelW+12,24.f,1256.f-LabelW);
    }
    Labels.Add(FBox2D(FVector2D(X-5,Y-4),FVector2D(X+LabelW+5,Y+Px*1.35f)));
    auto Shadow=Ink;Shadow.A=Fade;for(FVector2D D:{FVector2D(-1.5f,0),FVector2D(1.5f,0),FVector2D(0,-1.5f),FVector2D(0,1.5f)})Text(Label,X+D.X,Y+D.Y,Px,Shadow,true);
    auto Gold=Hex(TEXT("FFE49B"));Gold.A=Fade;Text(Label,X,Y,Px,Gold,true);
   }
  }
 };

 if(G->Stage==6){
  // Editorial left panel gives the live cat vignette room to breathe.
  DrawRect(Paper,0,0,OX+536*S,H);
  Box(44,44,32,4,Orange);Text(TEXT("DELIVERY CHAOS"),88,36,16,Ink,true);
  if(!TitleMark)TitleMark=LoadObject<UTexture2D>(nullptr,TEXT("/Game/UI/Title/T_TitleMark"));
  if(TitleMark){const float LogoW=470,LogoH=LogoW*TitleMark->GetSizeY()/FMath::Max(1,TitleMark->GetSizeX());DrawTexture(TitleMark,OX+24*S,OY+74*S,LogoW*S,LogoH*S,0,0,1,1,FLinearColor::White,BLEND_Translucent);}
  else Text(TEXT("猫手猫脚"),44,177,62,Ink,true);
  Button(TEXT("TitleStart"),TEXT("开始捣蛋"),TEXT("Enter"),48,445,320,true);
  Button(TEXT("TitleFree"),TEXT("随便拆拆"),G->FocusedMenuAction()==TEXT("TitleFree")?TEXT("Enter"):TEXT("F"),48,510,320,false);
  Text(TEXT("方向键选择 · 回车开始"),50,576,14,Muted);
  Box(48,636,440,1,Line);
  const bool Hot=Hover(48,653,150,30);
  Text(G->MusicMuted?TEXT("音乐  /  关  M"):TEXT("音乐  /  开  M"),48,655,16,Hot?Orange:Ink,true);Hit(TEXT("TitleMusic"),48,648,150,40);

  return;
 }
 // Selection and fusion share a collection grid, with a distinct primary action.
 if(G->Stage==4||G->Stage==5){
  Background();Brand();const bool Lab=G->Stage==5;const int Count=G->Progress?FMath::CountBits((uint32)G->Progress->UnlockedMask):1;
  const int Made=G->Progress?FMath::CountBits((uint32)G->Progress->RecipesMask):0;
  Text(Lab?TEXT("合成"):TEXT("选择猫咪"),44,55,42,Ink,true);
  Pill(FString::Printf(TEXT("找到 %d / 4 只猫"),Count),933,40,Ink,Paper);Pill(FString::Printf(TEXT("合成 %d / 6"),Made),1110,40,Blue,Ink);
  Round(44,159,408,445,20,Ink);
  if(!Lab){
   Pill(TraitName(G->SelectedCat),66,177,Orange,Ink);
   Pic(G->SelectedCat,111,218,274);
   Text(LabCatalog::Cat(G->SelectedCat).Name,68,512,34,Paper,true);
  }else{
   Pill(TEXT("这次的组合"),66,177,Orange,Ink);
   Pic(G->SelectedCat,70,227,152);if(G->PartnerCat>=0)Pic(G->PartnerCat,274,227,152);
   Center(TEXT("＋"),248,283,32,Orange,true);
   Center(LabCatalog::Cat(G->SelectedCat).Name,146,391,19,Paper);
   if(G->PartnerCat>=0)Center(LabCatalog::Cat(G->PartnerCat).Name,350,391,19,Paper);
   int R=LabCatalog::RecipeFor(G->SelectedCat,G->PartnerCat);bool Ready=R>=0&&G->RecipeUnlocked(R);
   Box(68,434,360,1,Hex(TEXT("4B4A43")));
   Text(Ready?LabCatalog::Recipe(R).Name:TEXT("这个组合还没解锁。"),68,457,Ready?30:23,Ready?Orange:Paper,true);
   FString Detail=R>=0?(Ready?LabCatalog::Recipe(R).Description:LabCatalog::Recipe(R).Hint):TEXT("先选另一只已经找到的猫。");
   if(!Ready&&R==5){const int Other=G->Progress?FMath::CountBits((uint32)(G->Progress->RecipesMask&31)):0;Detail=FString::Printf(TEXT("已找到 %d / 4 只猫\n已合成 %d / 3 种其他组合"),Count,FMath::Min(3,Other));}
   else if(!Ready&&R==4){const int Ordinary=G->Progress?FMath::CountBits((uint32)(G->Progress->RecipesMask&15)):0;Detail=FString::Printf(TEXT("先试两种普通组合。\n目前完成 %d / 2 种"),FMath::Min(2,Ordinary));}
   Wrap(Detail,68,511,17,356,Paper);
  }
  for(int I=0;I<4;I++){
   const float X=480+(I%2)*386,Y=159+(I/2)*165;const bool Known=G->Unlocked(I),Main=Lab&&I==G->SelectedCat,Chosen=I==(Lab?G->PartnerCat:G->SelectedCat);
   const bool Hot=Known&&!Main&&Hover(X,Y,370,149);
   Round(X,Y,370,149,16,Chosen?Orange:Hot?Blue:Line);Round(X+2,Y+2,366,145,14,Chosen?White:Paper);
   if(Known)Pic(I,X+14,Y+19,110);else{CatMark(X+69,Y+66,38,Line);Center(TEXT("?"),X+69,Y+57,25,Muted,true);}
   const FString Name=Known?LabCatalog::Cat(I).Name:TEXT("还没找到");Text(Name,X+140,Y+22,24,Known?Ink:Muted,true);
   Text(Known?TraitName(I):TEXT("藏猫线索"),X+140,Y+62,15,Muted);
   Fit(Known?(Main?TEXT("已选主猫"):Chosen?TEXT("选好了"):Lab?TEXT("选作搭档"):TEXT("选我出门")):Clue(I),X+140,Y+103,15,213,Chosen?Hex(TEXT("A13A0F")):Muted);
   if(Known)Key(FString::FromInt(I+1),X+320,Y+21,30);
   if(Known&&!Main)Hit(FName(*FString::Printf(TEXT("Cat%d"),I)),X,Y,370,149);
  }
  // The next useful information sits beside the choice, not in an unrelated banner.
  if(Lab){
   Button(TEXT("Solo"),TEXT("单猫出门"),TEXT("Space"),998,527,218,false);
  }else{
   Round(480,501,756,103,16,Blue);
   Text(LabCatalog::Cat(G->SelectedCat).Ability,500,518,18,Ink,true);
   const bool BigKnown=G->Progress&&(G->Progress->RecipesMask&(1<<5));
   Text(BigKnown?TEXT("大坏猫配方  阿坏 + 大福"):
    TEXT("隐藏猫 · 收集 4 猫，完成 3 种合成"),500,556,15,Ink);
  }
  if(Lab){
   Button(TEXT("Select"),TEXT("重选主猫"),TEXT("Tab"),44,628,236,false);
   Text(G->Practice?TEXT("自由拆家 · 不限时，不计榜"):TEXT("限时拆家 · 180 秒挑战"),310,646,16,Muted);
   const int R=LabCatalog::RecipeFor(G->SelectedCat,G->PartnerCat);const bool Ready=R>=0&&G->Unlocked(G->PartnerCat)&&G->RecipeUnlocked(R);
   Button(TEXT("Confirm"),Ready?TEXT("合成，去拆家"):TEXT("先完成解锁条件"),TEXT("Enter"),918,628,318,true,Ready);
  }else{
   Button(TEXT("Practice"),G->Practice?TEXT("自由拆家"):TEXT("限时拆家"),TEXT("F"),44,628,236,false);
   Text(G->Practice?TEXT("慢慢逛，不限时 · 不计榜"):TEXT("180 秒挑战 · 计入排行榜"),300,646,16,Muted);
   Button(TEXT("Confirm"),Count>1?TEXT("选搭档"):TEXT("开始拆家"),TEXT("Enter"),918,628,318,true);
  }
  Footer(Lab?TEXT("方向键选搭档    Space 单猫出门    Tab 重选主猫"):TEXT("方向键选猫    Enter 确认    F 切换模式"));return;
 }

 const FString Identity=G->RecipeId>=0?LabCatalog::Recipe(G->RecipeId).Name:LabCatalog::Cat(G->SelectedCat).Name;
 if(G->Stage==1){
  DrawRect(Ink,0,0,W,H);Box(0,0,1280,8,Orange);
  Text(TEXT("合成"),44,35,16,Paper);
  float T=FMath::Clamp(1-G->Remaining/4,0.f,1.f),Ease=T*T*(3-2*T);
  Center(TEXT("合成中"),640,105,33,Paper,true);
  Round(218+Ease*65,190,250,250,20,Paper);Round(812-Ease*65,190,250,250,20,Paper);
  Pic(G->SelectedCat,233+Ease*65,205,220);Pic(G->PartnerCat,827-Ease*65,205,220);
  Center(TEXT("＋"),640,279,52,Orange,true);
  Center(Identity,640,484,48,Orange,true);Center(LabCatalog::Recipe(G->RecipeId).Description,640,552,20,Paper);
  Round(440,622,400,6,3,Hex(TEXT("4B4A43")));if(T>0)Round(440,622,400*T,6,3,Orange);
  if(!Paused)return;
 }

 if(G->Stage==3&&G->MemeShowing&&!Paused){
  Box(0,0,1280,143,Ink);Box(0,586,1280,134,Ink);
  Center(TEXT("喵。"),640,28,38,Paper,true);
  Center(Identity,640,86,22,Orange,true);

  Text(TEXT("本次损失"),1030,272,18,Paper);Fit(Money(G->Score),1030,308,32,198,Orange,true);
  Button(TEXT("Meme"),TEXT("好了，看账单"),TEXT("Space"),446,650,388,true);
  return;
 }

 if(G->Stage==3&&!Paused){
  DrawRect(Ink,0,0,W,H);Box(0,0,1280,8,Orange);
  Text(TEXT("结算"),64,35,34,Paper,true);
  // Receipt with a torn edge, real damage rows and a truthful zero-damage state.
  Round(64,104,662,507,18,White);for(int I=0;I<33;I++)Tri(FVector2D(66+I*20,605),FVector2D(86+I*20,605),FVector2D(76+I*20,617),White);
  Text(TEXT("本次拆家账单"),92,128,20,Muted);Text(Identity,532,132,17,Muted);
  Fit(Money(G->Score),92,173,72,594,Ink,true);
  for(int I=0;I<30;I++)Box(92+I*20,307,10,1,Line);
  TMap<FString,int32> Totals;int Broken=0;for(auto P:G->Props)if(IsValid(P)&&P->Scored){Totals.FindOrAdd(P->ItemName)+=P->BaseScore;Broken++;}
  TArray<FString> Names;Totals.GetKeys(Names);Names.Sort([&](const FString& A,const FString& B){return Totals[A]>Totals[B];});
  int Shown=0;for(int I=0;I<FMath::Min(3,Names.Num());I++){Text(Names[I],94,331+I*37,18,Ink);FString Price=Money(Totals[Names[I]]);Text(Price,694-Width(Price,20),331+I*37,20,Ink);Shown+=Totals[Names[I]];}
  if(Names.Num()>3){Text(TEXT("其余损失"),94,442,16,Muted);FString Price=Money(G->Score-Shown);Text(Price,694-Width(Price,18),442,18,Muted);}
  if(Names.IsEmpty())Text(TEXT("暂无损失"),94,350,18,Muted);
  Box(92,480,602,1,Line);
  Text(FString::Printf(TEXT("碰坏 %d 件"),Broken),94,502,18,Ink,true);Text(FString::Printf(TEXT("出爪 %d 次"),G->Swipes),300,502,18,Ink,true);Text(FString::Printf(TEXT("装乖 %d 次"),G->GoodCat),510,502,18,Ink,true);
  Round(754,104,462,123,18,G->PerfectCrime?Green:Blue);
  Text(G->Score==0?TEXT("零损失"):G->PerfectCrime?TEXT("完美犯罪"):G->Suspicion>=3?TEXT("当场抓包"):TEXT("证据不足"),778,123,32,Ink,true);
  Round(754,247,462,364,18,Hex(TEXT("30322F")));
  Text(TEXT("本机拆家榜"),778,269,24,Paper,true);if(G->Practice)Text(TEXT("自由模式不计榜"),778,310,16,Hex(TEXT("BFC3B7")));
  if(G->Progress&&!G->Progress->Board.IsEmpty())for(int I=0;I<G->Progress->Board.Num();I++){
   const auto& Row=G->Progress->Board[I];float Y=354+I*45;Text(FString::Printf(TEXT("%02d"),I+1),780,Y,18,I==0?Orange:Paper,true);
   Fit(Row.Cat,824,Y,17,192,Paper);const FString Price=Money(Row.Money);Text(Price,1190-Width(Price,20),Y,20,I==0?Orange:Paper,true);
  }else Wrap(TEXT("暂无记录"),780,367,19,392,Paper);
  Button(TEXT("Select"),G->Progress&&FMath::CountBits((uint32)G->Progress->UnlockedMask)>1?TEXT("换个组合再来"):TEXT("回去选猫"),TEXT("Enter"),64,638,400,true);Button(TEXT("Retry"),TEXT("原样再拆一局"),G->FocusedMenuAction()==TEXT("Retry")?TEXT("Enter"):TEXT("R"),484,638,330,false);
  Button(TEXT("Meme"),TEXT("猫猫彩蛋"),G->FocusedMenuAction()==TEXT("Meme")?TEXT("Enter"):TEXT("Space"),834,638,382,false);
  if(G->MemeGifShowing&&G->MemeGifTexture){
   DrawRect(FLinearColor(0,0,0,.7f),0,0,W,H);Round(420,94,440,530,20,Paper);Center(TEXT("猫猫彩蛋"),640,120,26,Ink,true);
   int Frame=FMath::FloorToInt(G->MemeGifElapsed*12)%32;
   DrawTexture(G->MemeGifTexture,OX+480*S,OY+180*S,320*S,320*S,(Frame%8)/8.f,(Frame/8)/4.f,1.f/8,1.f/4,FLinearColor::White,BLEND_Translucent);
   Button(TEXT("Meme"),TEXT("收起"),TEXT("Space"),470,548,340,true);
  }return;
 }

 if(G->Playing()&&!G->HideHUD&&!Paused){
  // Leave the middle of the room open. Only actionable information stays on screen.
  Round(24,22,244,88,15,Ink);Text(TEXT("拆家账单"),42,36,15,Paper);
  Fit(Money(FMath::RoundToInt(G->DisplayBill)),42,61,33+G->BillPulse*3,208,FMath::Lerp(Orange,Hex(TEXT("FFD970")),G->BillPulse),true);
  Round(512,22,256,76,15,G->Remaining<=10&&!G->Practice?Red:Ink);
  Center(G->Practice?TEXT("自由模式"):FString::Printf(TEXT("%02d 秒"),FMath::CeilToInt(G->Remaining)),640,32,31,Paper,true);
  Center(G->Practice?TEXT("不限时 · 不计榜"):TEXT("派对倒计时"),640,73,13,Paper);
  Round(956,22,300,78,15,G->CameraBroken?Hex(TEXT("283B2C")):Ink);
  Ellipse(976,46,4,4,G->CameraBroken?Green:G->CameraFlash>0?Red:Orange);
  Text(G->CameraBroken?TEXT("监控已断线"):G->CameraFlash>0?TEXT("被拍到了！"):G->Exposure>0?TEXT("快躲开"):G->CameraInView?TEXT("保持淡定"):TEXT("监控巡查"),990,35,19,Paper,true);
  Round(974,70,264,5,2,Hex(TEXT("62645E")));
  if(G->Exposure>0)Round(974,70,264*G->Exposure/G->CameraNoticeTime,5,2,Orange);

  Round(24,122,244,38,10,Paper);Text(TEXT("超级扑"),40,131,16,Ink,true);
  for(int I=0;I<3;I++)CatMark(147+I*36,141,9,I<G->Pounces?Orange:Line);

  const FString Toast=Now<G->QuipUntil?G->Quip:Now<CaptionVisibleUntil?G->CameraCaption:
   Now-StageEnteredAt<8&&G->Swipes==0?TEXT("靠近杯子，按 J。"):TEXT("");
  if(!Toast.IsEmpty()){Round(370,117,540,68,12,Paper);Wrap(Toast,392,132,18,496,Ink);}

  if(Cat){
   float Grab=0;for(auto R:G->Residents)if(IsValid(R))Grab=FMath::Max(Grab,R->CaptureTime);
   if(Grab>0){const FVector Mark=Project(Cat->GetActorLocation()+FVector(0,0,85));if(Mark.Z>0){const FVector2D At((Mark.X-OX)/S,(Mark.Y-OY)/S);Round(At.X-46,At.Y-28,92,35,10,Ink);Center(TEXT("快跑！"),At.X,At.Y-22,16,Paper,true);Round(At.X-34,At.Y+1,68*Grab/.5f,3,1,Orange);}}
   if(Cat->Charging){Round(470,526,340,67,12,Ink);Text(Cat->Charge>=1.05&&G->Pounces>0?TEXT("超级扑就绪，松开释放"):TEXT("松开扑击"),490,538,19,Paper);Round(490,574,300,5,2,Muted);if(Cat->Charge>0)Round(490,574,300*Cat->Charge/1.25f,5,2,Orange);}
   else if(Cat->Innocent){Pill(TEXT("装乖中"),512,539,Paper,Ink);}
   if(auto P=Cat->FindTarget(170,.1f);P){
    const FVector Mark=Project(P->GetActorLocation()+FVector(0,0,35));if(Mark.Z>0){FVector2D At((Mark.X-OX)/S,(Mark.Y-OY)/S);Tri(At+FVector2D(-7,-15),At+FVector2D(7,-15),At+FVector2D(0,-5),Orange);}
    FString Label=P->ItemName+TEXT("  ")+Money(P->BaseScore);float BW=FMath::Max(390.f,Width(Label,17)+210);float X=(1280-BW)*.5f;Round(X,607,BW,40,10,Paper);Text(Label,X+16,618,17,Ink);Key(TEXT("J"),X+BW-170,614,26);Text(TEXT("拍"),X+BW-138,619,15,Ink);Key(TEXT("E"),X+BW-91,614,26);Text(TEXT("推"),X+BW-59,619,15,Ink);
   }
  }
  if(Now-StageEnteredAt<12){Round(24,668,960,34,9,Ink);Text(TEXT("方向键 / WASD 移动    J 拍 · 长按扑    Space 跳    E 推"),40,677,14,Paper);
  if(G->CurrentTraits()&CatTrait::Loud)Text(TEXT("Q 喵一声"),858,677,14,Orange);}
  Round(1064,668,192,34,9,Ink);Text(TEXT("Esc 暂停与操作说明"),1080,677,14,Paper);
  MoneyFeedback();
 }
 if(Paused){
  DrawRect(FLinearColor(0,0,0,.82f),0,0,W,H);Round(160,111,960,498,22,Paper);
  Text(TEXT("暂停"),192,141,36,Ink,true);
  const TCHAR* Keys[]={TEXT("方向键"),TEXT("J"),TEXT("长按 J"),TEXT("E"),TEXT("Shift"),TEXT("Space"),TEXT("Q")};
  const TCHAR* Help[]={TEXT("移动，也可用 WASD"),TEXT("挥爪"),TEXT("蓄力后松开扑击"),TEXT("慢推面前的物件"),TEXT("暴冲"),TEXT("跳跃"),TEXT("音波，需要乌梅的能力")};
  for(int I=0;I<7;I++){Key(Keys[I],194,245+I*39,86);Text(Help[I],294,249+I*39,17,Ink);}
  Text(TEXT("停下会自动装乖，能骗过摄像头。"),194,537,16,Muted);
  Text(TEXT("H 隐藏界面    C 切换远近"),194,571,14,Muted);
  Button(TEXT("Resume"),TEXT("继续拆家"),TEXT("Enter"),742,245,344,true);
  Button(TEXT("Retry"),TEXT("重新开始这局"),TEXT("R"),742,311,344,false);
  Button(TEXT("Select"),TEXT("返回选猫"),TEXT("Tab"),742,377,344,false);
  Text(TEXT("方向键选择 · 回车确认"),744,447,15,Muted);
  Button(TEXT("Shake"),Cat&&Cat->Shake?TEXT("镜头震动已开启"):TEXT("镜头震动已关闭"),TEXT("V"),742,493,344,false);
 }
}
