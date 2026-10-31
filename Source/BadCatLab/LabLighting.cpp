#include "LabGame.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/GameViewportClient.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "HAL/IConsoleManager.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace {
void SetRender(const TCHAR* Name,int Value){if(auto V=IConsoleManager::Get().FindConsoleVariable(Name))V->Set(Value,ECVF_SetByConsole);}
int RenderValue(const TCHAR* Name){auto V=IConsoleManager::Get().FindConsoleVariable(Name);return V?V->GetInt():-1;}
}
void ALabGameMode::ConfigureLighting(){
#if !UE_BUILD_SHIPPING
 FParse::Value(FCommandLine::Get(),TEXT("LightingStage="),LightingStage);LightingStage=FMath::Clamp(LightingStage,0,4);
 FParse::Value(FCommandLine::Get(),TEXT("LumenQuality="),LightingQuality);LightingQuality=FMath::Clamp(LightingQuality,0,2);
 FParse::Value(FCommandLine::Get(),TEXT("VisualBench="),VisualLabel);
 if(!VisualLabel.IsEmpty()){
  VisualLabel=FPaths::GetCleanFilename(VisualLabel);SetRender(TEXT("r.VSync"),0);SetRender(TEXT("t.MaxFPS"),0);
 }
 if(LightingStage==0){SetRender(TEXT("r.DynamicGlobalIlluminationMethod"),0);SetRender(TEXT("r.ReflectionMethod"),0);SetRender(TEXT("r.DistanceFieldAO"),0);}
#endif
 SetRender(TEXT("sg.GlobalIlluminationQuality"),LightingQuality);SetRender(TEXT("sg.ReflectionQuality"),LightingQuality);
 UE_LOG(LogTemp,Display,TEXT("Lighting stage %d, GI=%d, Reflection=%d, MeshDF=%d, HWRT=%d, quality=%d"),LightingStage,RenderValue(TEXT("r.DynamicGlobalIlluminationMethod")),RenderValue(TEXT("r.ReflectionMethod")),RenderValue(TEXT("r.GenerateMeshDistanceFields")),RenderValue(TEXT("r.Lumen.HardwareRayTracing")),LightingQuality);
}
void ALabGameMode::BuildLightingFinish(){
 auto W=GetWorld();
 if(LightingStage>=2){
  auto WarmLight=[&](FVector P,float Lumens,float Radius){auto A=W->SpawnActor<APointLight>(P,FRotator::ZeroRotator);auto C=CastChecked<UPointLightComponent>(A->GetLightComponent());C->SetMobility(EComponentMobility::Movable);C->SetIntensityUnits(ELightUnits::Lumens);C->SetIntensity(Lumens);C->SetLightColor(FLinearColor(1,.45f,.16f));C->SetAttenuationRadius(Radius);C->SetSourceRadius(8);C->SetCastShadows(true);A->Tags.Add(TEXT("WarmPractical"));};
  WarmLight(FVector(-650,550,275),1800,420); // Actual bedside lamp.
  WarmLight(FVector(-40,615,60),1300,360);   // Pumpkin fireplace.
 }
 if(LightingStage>=3){
  auto P=W->SpawnActor<APostProcessVolume>();P->bUnbound=true;P->Priority=10;P->Tags.Add(TEXT("WarmGrade"));auto& S=P->Settings;
  S.bOverride_ColorSaturation=true;S.ColorSaturation=FVector4(1,1,1,1.15f);
  S.bOverride_ColorContrast=true;S.ColorContrast=FVector4(1,1,1,1.1f);
  S.bOverride_TemperatureType=true;S.TemperatureType=TEMP_ColorTemperature;S.bOverride_WhiteTemp=true;S.WhiteTemp=5800;
  S.bOverride_BloomIntensity=true;S.BloomIntensity=.5f;
  S.bOverride_VignetteIntensity=true;S.VignetteIntensity=.3f;
 }
 if(LightingStage>=4){
  auto A=W->SpawnActor<AExponentialHeightFog>(FVector(0,0,-100),FRotator::ZeroRotator);auto C=A->GetComponent();C->SetFogDensity(.003f);C->SetFogHeightFalloff(.2f);C->SetFogInscatteringColor(FLinearColor(.55f,.32f,.17f));C->SetStartDistance(200);C->SetFogMaxOpacity(.12f);C->SetVolumetricFog(false);A->Tags.Add(TEXT("WarmHaze"));
 }
}
void ALabGameMode::AddWarmPropLight(ALabProp* Prop){
 if(LightingStage<2||!IsValid(Prop))return;
 auto C=NewObject<UPointLightComponent>(Prop);C->SetupAttachment(Prop->Body);C->SetAbsolute(false,false,true);C->RegisterComponent();C->SetWorldLocation(Prop->GetActorLocation()+FVector(0,0,40));C->SetIntensityUnits(ELightUnits::Lumens);C->SetIntensity(Prop->PropId==13?1400:900);C->SetLightColor(FLinearColor(1,.45f,.16f));C->SetAttenuationRadius(Prop->PropId==13?360:280);C->SetSourceRadius(8);C->SetCastShadows(true);
}
void ALabGameMode::TickVisualBenchmark(){
#if !UE_BUILD_SHIPPING
 if(VisualLabel.IsEmpty()||VisualComplete)return;
 const double Now=FPlatformTime::Seconds();const double Delta=VisualLastTime>0?Now-VisualLastTime:0;VisualLastTime=Now;
 if(!FApp::HasFocus()||Delta<=0)return; // Never count background-throttled frames.
 if(VisualWarmup<15){VisualWarmup+=Delta;return;}
 VisualFrameTimes.Add(Delta);VisualSampleSeconds+=Delta;const double Total=VisualSampleSeconds;if(Total<30)return;
 VisualComplete=true;TArray<double> Sorted=VisualFrameTimes;Sorted.Sort();const double P99=Sorted[FMath::Clamp(FMath::FloorToInt(Sorted.Num()*.99),0,Sorted.Num()-1)];
 double Window=0,Minimum5=100000;int Frames=0;for(double T:VisualFrameTimes){Window+=T;Frames++;if(Window>=5){Minimum5=FMath::Min(Minimum5,Frames/Window);Window=0;Frames=0;}}
 FVector2D Size(0,0);if(GetWorld()->GetGameViewport())GetWorld()->GetGameViewport()->GetViewportSize(Size);
 const double FPS=VisualFrameTimes.Num()/Total;
 FString Report=FString::Printf(TEXT("{\n  \"label\": \"%s\",\n  \"stage\": %d,\n  \"lumen_quality\": %d,\n  \"width\": %.0f,\n  \"height\": %.0f,\n  \"sample_seconds\": %.3f,\n  \"frames\": %d,\n  \"average_fps\": %.2f,\n  \"p99_frame_ms\": %.2f,\n  \"minimum_5s_fps\": %.2f,\n  \"foreground_only\": true,\n  \"vsync\": %d,\n  \"screen_percentage\": %d,\n  \"gi_method\": %d,\n  \"reflection_method\": %d,\n  \"mesh_distance_fields\": %d,\n  \"hardware_ray_tracing\": %d\n}\n"),*VisualLabel,LightingStage,LightingQuality,Size.X,Size.Y,Total,VisualFrameTimes.Num(),FPS,P99*1000,Minimum5,RenderValue(TEXT("r.VSync")),RenderValue(TEXT("r.ScreenPercentage")),RenderValue(TEXT("r.DynamicGlobalIlluminationMethod")),RenderValue(TEXT("r.ReflectionMethod")),RenderValue(TEXT("r.GenerateMeshDistanceFields")),RenderValue(TEXT("r.Lumen.HardwareRayTracing")));
 const auto Folder=FPaths::ProjectSavedDir()/TEXT("VisualBench");IFileManager::Get().MakeDirectory(*Folder,true);FFileHelper::SaveStringToFile(Report,*(Folder/(VisualLabel+TEXT(".json"))),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
 UE_LOG(LogTemp,Display,TEXT("Visual benchmark complete: %s average %.2f FPS, minimum 5s %.2f at %.0fx%.0f"),*VisualLabel,FPS,Minimum5,Size.X,Size.Y);
#endif
}
