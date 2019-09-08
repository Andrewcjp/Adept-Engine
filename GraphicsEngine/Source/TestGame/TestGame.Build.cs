using EngineBuildTool;

class TestGameModule : ModuleDef
{
    public TestGameModule(TargetRules Rules) : base(Rules)
    {
        ModuleName = "TestGame";
        SourceFileSearchDir = "TestGame";
        ModuleOutputType = ModuleDef.ModuleType.ModuleDLL;
        SolutionFolderPath = "Game";
        PCH = "TestGamePCH";
        UseCorePCH = false;
        IsGameModule = true;
        UseUnity = true;
    }
}