using EngineBuildTool;

class TestGameModule : ModuleDef
{
    public TestGameModule()
    {
        ModuleName = "TestGame";
        SourceFileSearchDir = "TestGame";
        ModuleDepends.Add("Core");
        ModuleOuputType = ModuleDef.ModuleType.DLL;
    }
}