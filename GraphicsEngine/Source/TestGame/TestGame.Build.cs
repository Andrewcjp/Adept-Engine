using EngineBuildTool;

class TestGameModule : ModuleDef
{
    public TestGameModule()
    {
        ModuleName = "TestGame";
        SourceFileSearchDir = "TestGame";
        ModuleDepends.Add("Core");
        ModuleOutputType = ModuleDef.ModuleType.DLL;
        SolutionFolderPath = "Game";
    }
}