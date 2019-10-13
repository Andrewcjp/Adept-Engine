using EngineBuildTool;

class CSTestGameModule : ModuleDef
{
    public CSTestGameModule(TargetRules Rules) : base(Rules)
    {
        SolutionFolderPath = "Game/CSharp";
        ModuleName = "CSTestGame";
        SourceFileSearchDir = "CSTestGame";
        PCH = "";
        ModuleOutputType = ModuleDef.ModuleType.DLL;
        UseCorePCH = false;
        UseUnity = false;
        NeedsCore = false;
        ModuleDepends.Add("CSharpCore");
        NetReferences.Add("CSharpCore");
        NetReferences.Add("CSharpBridge");
        LaunguageType = ProjectType.CSharp;
        UnsupportedPlatforms.Add("Android");
    }
}