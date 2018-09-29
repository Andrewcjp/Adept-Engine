using EngineBuildTool;

class TDPhysicsModule : ModuleDef
{
    public TDPhysicsModule()
    {
        SolutionFolderPath = "Engine/Physics";
        ModuleName = "TDPhysics";
        SourceFileSearchDir = "TDPhysics";
        PCH = "TDPCH";
        // ModuleDepends.Add("Core");
        ModuleOutputType = ModuleDef.ModuleType.DLL;
        UseCorePCH = false;
    }
}