using EngineBuildTool;

class TDPhysicsModule : ModuleDef
{
    public TDPhysicsModule(TargetRules Rules) : base(Rules)
    {
        SolutionFolderPath = "Engine/Physics";
        ModuleName = "TDPhysics";
        SourceFileSearchDir = "TDPhysics";
        PCH = "TDPCH";
        // ModuleDepends.Add("Core");
        NeedsCore = false;
        ModuleOutputType = ModuleDef.ModuleType.DLL;
        UseCorePCH = false;
        UseUnity = true;
    }
}