using EngineBuildTool;

class GameModule : ModuleDef
{
    public GameModule(TargetRules Rules) : base(Rules)
    {
        ModuleName = "Game";
        SourceFileSearchDir = "Game";
        ModuleOutputType = ModuleDef.ModuleType.EXE;
        SolutionFolderPath = "Game";
        PCH = "";
        UseCorePCH = false;
        IsGameModule = false;
        IsOutputEXE = true;
        UseUnity = false;
    }
}