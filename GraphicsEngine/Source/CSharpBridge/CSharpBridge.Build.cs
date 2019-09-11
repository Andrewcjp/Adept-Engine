using EngineBuildTool;

class CSharpBridgeModule : ModuleDef
{
    public CSharpBridgeModule(TargetRules Rules) : base(Rules)
    {
        SolutionFolderPath = "Engine/CSharp";
        ModuleName = "CSharpBridge";
        SourceFileSearchDir = "CSharpBridge";//CoreModules/
        PCH = "CSharpBridgePCH";
        ModuleOutputType = ModuleDef.ModuleType.ModuleDLL;
        UseCorePCH = false;
        UseUnity = true;

        LaunguageType = ProjectType.ManagedCPP;
    }
}