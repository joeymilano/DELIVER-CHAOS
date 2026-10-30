using UnrealBuildTool;
public class BadCatLab : ModuleRules { public BadCatLab(ReadOnlyTargetRules Target):base(Target) { PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs; PublicDependencyModuleNames.AddRange(new string[]{"Core","CoreUObject","Engine","InputCore","PhysicsCore","SlateCore"}); } }
