using EngineBuildTool;

class CSharpContainerModule : ModuleDef
{
    public CSharpContainerModule(TargetRules Rules) : base(Rules)
    {
        SolutionFolderPath = "Engine/CSharp";
        ModuleName = "CSharpContainer";
        SourceFileSearchDir = "CoreModules/CSharpContainer";//CoreModules/
        PCH = "CSharpContainerPCH";
        ModuleOutputType = ModuleDef.ModuleType.ModuleDLL;
        UseCorePCH = false;
        UseUnity = true;
        NetReferences.Add("CSharpCore");
        ModuleDepends.Add("CSharpCore");
        IncludeDirectories.Add("//Source//CoreModules//CSharpBridge//");
        LaunguageType = ProjectType.ManagedCPP;
        UnsupportedPlatforms.Add("Android");
    }
}