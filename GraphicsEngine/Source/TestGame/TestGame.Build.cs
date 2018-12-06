using EngineBuildTool;

class TestGameModule : ModuleDef
{
    public TestGameModule()
    {
        ModuleName = "TestGame";
        SourceFileSearchDir = "TestGame";
        ModuleOutputType = ModuleDef.ModuleType.ModuleDLL;
        SolutionFolderPath = "Game";
        PCH = "TestGamePCH";
        UseCorePCH = false;
        IsGameModule = true;
    }
}