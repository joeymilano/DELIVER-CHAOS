#pragma once
#include "CoreMinimal.h"
namespace CatTrait { constexpr uint8 Wild=1, Heavy=2, Loud=4, Giant=8; }
struct FLabCatDefinition {
 const TCHAR* Name; const TCHAR* Breed; const TCHAR* Motto; const TCHAR* Portrait; const TCHAR* Mesh;
 FLinearColor Color; float Speed; float Jump; float Swipe; uint8 Traits;
};
struct FLabRecipe {
 const TCHAR* Name; const TCHAR* Description; const TCHAR* Hint; FLinearColor Color;
 bool Bounce; bool Resonance; float BounceImpulse; int32 PulseTargets; uint8 Traits; bool Hidden;
};
namespace LabCatalog {
 constexpr int32 CatCount=4, RecipeCount=6;
 const FLabCatDefinition& Cat(int32 Index);
 int32 RecipeFor(int32 A,int32 B);
 const FLabRecipe& Recipe(int32 Index);
}
