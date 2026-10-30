#pragma once
#include "CoreMinimal.h"

// Recipe identity is unordered. A recipe changes simulation rules, not only the skin.
struct FLabCatDefinition
{
 const TCHAR* Name;
 const TCHAR* Breed;
 const TCHAR* Motto;
 const TCHAR* Portrait;
 const TCHAR* Mesh;
 FLinearColor Color;
 float Speed;
 float Jump;
 float Swipe;
};
struct FLabRecipe
{
 const TCHAR* Name;
 const TCHAR* Description;
 const TCHAR* Hint;
 FLinearColor Color;
 bool Bounce;
 bool Resonance;
 float BounceImpulse;
 int32 PulseTargets;
};
namespace LabCatalog
{
 constexpr int32 CatCount=3;
 const FLabCatDefinition& Cat(int32 Index);
 int32 RecipeFor(int32 A,int32 B);
 const FLabRecipe& Recipe(int32 Index);
}
