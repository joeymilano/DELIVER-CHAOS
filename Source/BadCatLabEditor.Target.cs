using UnrealBuildTool;
public class BadCatLabEditorTarget : TargetRules { public BadCatLabEditorTarget(TargetInfo Target) : base(Target) { Type=TargetType.Editor; DefaultBuildSettings=BuildSettingsVersion.V6; IncludeOrderVersion=EngineIncludeOrderVersion.Latest; ExtraModuleNames.Add("BadCatLab"); } }
