#include "LabCatAnimation.h"
#include "LabGame.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimationPoseData.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "TwoBoneIK.h"

namespace {
struct FLabCatAnimationProxy : FAnimInstanceProxy {
 explicit FLabCatAnimationProxy(UAnimInstance* Instance) : FAnimInstanceProxy(Instance) {}
 UAnimSequence* Clip = nullptr;
 int32 State = INDEX_NONE, Serial = INDEX_NONE;
 float Time = 0, Transition = 1, BlendDuration = .16f, Step = 0, Reach = 0;
 FVector PawTarget = FVector::ZeroVector;
 TArray<FTransform> DisplayedPose, OutgoingPose;

 virtual void Initialize(UAnimInstance* Instance) override {
  FAnimInstanceProxy::Initialize(Instance);
  State=Serial=INDEX_NONE;DisplayedPose.Empty();OutgoingPose.Empty();Transition=1;
 }
 virtual void PreUpdate(UAnimInstance* Instance,float Dt) override {
  FAnimInstanceProxy::PreUpdate(Instance,Dt);
  auto Cat=Cast<ALabCat>(Instance->GetOwningActor());
  if(!Cat)return;
  const int32 Next=Cat->ActiveAnimation;
  if(Next!=State||Serial!=Cat->AnimationSerial){
   OutgoingPose=DisplayedPose;Transition=0;
   BlendDuration=Next==5||Next==7?.07f:Next==3?.26f:Next==6?.14f:.16f;
   State=Next;Serial=Cat->AnimationSerial;
  }
  Clip=Cat->Locomotion.IsValidIndex(State)?Cat->Locomotion[State]:nullptr;
  Time=Cat->AnimationTime;Step=Dt;Reach=Cat->PawReachAlpha;
  PawTarget=Cat->GetMesh()->GetComponentTransform().InverseTransformPosition(Cat->PawTargetWorld);
 }
 virtual bool Evaluate(FPoseContext& Output) override {
  Output.ResetToRefPose();
  if(!Clip)return true;
  FAnimationPoseData PoseData(Output);
  Clip->GetAnimationPose(PoseData,FAnimExtractContext(FMath::Clamp(double(Time),0.,double(Clip->GetPlayLength())),false));
  Transition=FMath::Min(1.f,Transition+Step/FMath::Max(.001f,BlendDuration));
  const float Alpha=FMath::SmoothStep(0.f,1.f,Transition);
  if(OutgoingPose.Num()==Output.Pose.GetNumBones()&&Alpha<1){
   for(auto I:Output.Pose.ForEachBoneIndex()){
    FTransform Blended;Blended.Blend(OutgoingPose[I.GetInt()],Output.Pose[I],Alpha);Output.Pose[I]=Blended;
   }
  }
  // Match only a reachable surface. Keep limb lengths; unreachable props never pull the rig apart.
  if(Reach>KINDA_SMALL_NUMBER){
   const auto& Bones=Output.Pose.GetBoneContainer();
   const auto& Ref=Bones.GetReferenceSkeleton();
   auto Index=[&](const TCHAR* Name){const int32 MeshIndex=Ref.FindBoneIndex(Name);return MeshIndex==INDEX_NONE?FCompactPoseBoneIndex(INDEX_NONE):Bones.MakeCompactPoseIndex(FMeshPoseBoneIndex(MeshIndex));};
   const auto Upper=Index(TEXT("RightArm")),Lower=Index(TEXT("RightForeArm")),Foot=Index(TEXT("RightFrontFoot"));
   if(Upper.GetInt()>=0&&Lower.GetInt()>=0&&Foot.GetInt()>=0){
    FCSPose<FCompactPose> CS;CS.InitPose(Output.Pose);
    FTransform A=CS.GetComponentSpaceTransform(Upper),B=CS.GetComponentSpaceTransform(Lower),C=CS.GetComponentSpaceTransform(Foot);
    const FVector Goal=FMath::Lerp(C.GetLocation(),PawTarget,FMath::Clamp(Reach,0.f,1.f));
    const FVector Pole=B.GetLocation()+(B.GetLocation()-A.GetLocation()).GetSafeNormal()*20;
    AnimationCore::SolveTwoBoneIK(A,B,C,Pole,Goal,false,1.,1.);
    const auto Parent=Bones.GetParentBoneIndex(Upper);
    Output.Pose[Upper]=A.GetRelativeTransform(CS.GetComponentSpaceTransform(Parent));
    Output.Pose[Lower]=B.GetRelativeTransform(A);Output.Pose[Foot]=C.GetRelativeTransform(B);
   }
  }
  DisplayedPose.SetNumUninitialized(Output.Pose.GetNumBones());
  for(auto I:Output.Pose.ForEachBoneIndex())DisplayedPose[I.GetInt()]=Output.Pose[I];
  return true;
 }
};
}
FAnimInstanceProxy* ULabCatAnimation::CreateAnimInstanceProxy(){return new FLabCatAnimationProxy(this);}
void ULabCatAnimation::DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy){delete static_cast<FLabCatAnimationProxy*>(Proxy);}
