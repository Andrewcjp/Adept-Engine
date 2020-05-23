using EngineBuildTool;

class StandaloneShaderComplierModule : ModuleDef
{
    public StandaloneShaderComplierModule(TargetRules Rules) : base(Rules)
    {
        ModuleName = "StandaloneShaderComplier";
        SourceFileSearchDir = "StandaloneShaderComplier";
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