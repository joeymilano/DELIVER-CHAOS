using UnrealBuildTool;
public class BadCatLabTarget : TargetRules { public BadCatLabTarget(TargetInfo Target) : base(Target) { Type=TargetType.Game; DefaultBuildSettings=BuildSettingsVersion.V7; IncludeOrderVersion=EngineIncludeOrderVersion.Latest; ExtraModuleNames.Add("BadCatLab"); } }
