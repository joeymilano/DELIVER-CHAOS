#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "LabCatAnimation.generated.h"

// Native pose sampling keeps the paw animation and gameplay contact on one clock.
// No generated assets or animation blueprint are required at runtime.
UCLASS(Transient)
class BADCATLAB_API ULabCatAnimation : public UAnimInstance {
 GENERATED_BODY()
protected:
 virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
 virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) override;
};
