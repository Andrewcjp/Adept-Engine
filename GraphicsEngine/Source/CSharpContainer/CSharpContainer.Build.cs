using EngineBuildTool;

class CSharpContainerModule : ModuleDef
{
    public CSharpContainerModule(TargetRules Rules) : base(Rules)
    {
        SolutionFolderPath = "Engine/CSharp";
        ModuleName = "CSharpContainer";
        SourceFileSearchDir = "CSharpContainer";//CoreModules/
        PCH = "CSharpContainerPCH";
        ModuleOutputType = ModuleDef.ModuleType.ModuleDLL;
        UseCorePCH = false;
        UseUnity = true;
        NetReferences.Add("CSharpCore");
        ModuleDepends.Add("CSharpCore");
        LaunguageType = ProjectType.ManagedCPP;
    }
}