using EngineBuildTool;

class Unit_TestsModule : ModuleDef
{
    public Unit_TestsModule()
    {
        SolutionFolderPath = "Tests";
        ModuleName = "Unit_Tests";
        SourceFileSearchDir = "Unit_Tests";
       // PCH = "TDPCH";
        // ModuleDepends.Add("Core");
        NeedsCore = false;
        ModuleOutputType = ModuleDef.ModuleType.EXE;
        UseCorePCH = false;
        UseConsoleSubSystem = true;
    }
}