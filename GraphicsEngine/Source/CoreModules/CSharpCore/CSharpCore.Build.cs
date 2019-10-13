using EngineBuildTool;

class CSharpCoreModule : ModuleDef
{
    public CSharpCoreModule(TargetRules Rules) : base(Rules)
    {
        SolutionFolderPath = "Engine/CSharp";
        ModuleName = "CSharpCore";
        SourceFileSearchDir = "CoreModules/CSharpCore";
        PCH = "";
        ModuleOutputType = ModuleDef.ModuleType.DLL;
        UseCorePCH = false;
        UseUnity = false;
        NeedsCore = false;
        ModuleDepends.Add("CSharpBridge");
        NetReferences.Add("CSharpBridge");
        LaunguageType = ProjectType.CSharp;
        UnsupportedPlatforms.Add("Android");
    }
}