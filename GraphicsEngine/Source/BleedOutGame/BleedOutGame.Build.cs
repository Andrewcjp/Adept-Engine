using EngineBuildTool;

class BleedOutGameModule : ModuleDef
{
    public BleedOutGameModule(TargetRules Rules) : base(Rules)
    {
        ModuleName = "BleedOutGame";
        SourceFileSearchDir = "BleedOutGame";
        ModuleOutputType = ModuleDef.ModuleType.ModuleDLL;
        SolutionFolderPath = "Game";
        PCH = "BleedOutPCH";
        UseCorePCH = false;
        IsGameModule = true;
        UseUnity = true;
    }
}