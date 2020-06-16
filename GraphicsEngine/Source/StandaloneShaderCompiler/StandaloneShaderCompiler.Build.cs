using EngineBuildTool;

class StandaloneShaderCompilerModule : ModuleDef
{
    public StandaloneShaderCompilerModule(TargetRules Rules) : base(Rules)
    {
        ModuleName = "StandaloneShaderCompiler";
        SourceFileSearchDir = "StandaloneShaderCompiler";
        ModuleOutputType = ModuleDef.ModuleType.EXE;
        SolutionFolderPath = "Tools";
        PCH = "";
        UseCorePCH = false;
        IsGameModule = false;
        UseUnity = false;
        UnsupportedPlatforms.Add("X*");
        UseConsoleSubSystem = true;
        ExcludeConfigs.Add("*package*");
    }
}