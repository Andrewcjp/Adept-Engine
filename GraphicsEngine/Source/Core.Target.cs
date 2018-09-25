using EngineBuildTool;

class CoreTargetRules : TargetRules
{
    public CoreTargetRules()
    {
        LibSearchPaths.Add(new LibSearchPath("\\64", LibBuildConfig.General));
        LibSearchPaths.Add(new LibSearchPath("\\64\\Debug", LibBuildConfig.Debug));
        LibSearchPaths.Add(new LibSearchPath("\\64\\Release", LibBuildConfig.Optimized));
        ModuleExcludeList.Add("VulkanRHI");
    }
    public override ModuleDef GetCoreModule()
    {
        ModuleDef CoreModule = new ModuleDef();
        CoreModule.ModuleName = "Core";
        CoreModule.ModuleOutputType = ModuleDef.ModuleType.EXE;
        CoreModule.SolutionFolderPath = "Engine/Core";
        CoreModule.SourceFileSearchDir = "Core";
        CoreModule.PCH = "Stdafx";
        CoreModule.IncludeDirectories.Add("/Include");
        CoreModule.IncludeDirectories.Add("");
        CoreModule.IncludeDirectories.Add("/Include/freetype2");
        return CoreModule;
    }
}