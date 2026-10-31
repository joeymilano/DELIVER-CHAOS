#include "LabGame.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void ALabGameMode::WriteSceneAudit(){
#if !UE_BUILD_SHIPPING
 if(!NoSave||!FParse::Param(FCommandLine::Get(),TEXT("SceneAudit")))return;
 FString Folder=FPaths::ProjectSavedDir()/TEXT("ReleaseChecks");FParse::Value(FCommandLine::Get(),TEXT("ReleaseCheckDir="),Folder);
 IFileManager::Get().MakeDirectory(*Folder,true);
 FString CSV=TEXT("actor,component,asset,slots,missing_materials,visible,collision,x,y,z\n");int Count=0;
 for(TActorIterator<AActor> It(GetWorld());It;++It){
  TInlineComponentArray<UMeshComponent*> Meshes(*It);
  for(auto C:Meshes){
   UObject* Asset=nullptr;if(auto StaticBody=Cast<UStaticMeshComponent>(C))Asset=StaticBody->GetStaticMesh();else if(auto SkinnedBody=Cast<USkeletalMeshComponent>(C))Asset=SkinnedBody->GetSkeletalMeshAsset();
   if(!Asset)continue;int Missing=0;for(int I=0;I<C->GetNumMaterials();I++)if(!C->GetMaterial(I))Missing++;
   const FVector P=C->GetComponentLocation();const bool Visible=C->IsVisible()&&!It->IsHidden();
   CSV+=FString::Printf(TEXT("\"%s\",\"%s\",\"%s\",%d,%d,%d,%d,%.2f,%.2f,%.2f\n"),*It->GetName(),*C->GetName(),*Asset->GetPathName(),C->GetNumMaterials(),Missing,Visible?1:0,int(C->GetCollisionEnabled()),P.X,P.Y,P.Z);Count++;
  }
 }
 FFileHelper::SaveStringToFile(CSV,*(Folder/TEXT("scene-assets.csv")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
 UE_LOG(LogTemp,Display,TEXT("SCENE_AUDIT: %d mesh components recorded to %s"),Count,*Folder);
#endif
}
