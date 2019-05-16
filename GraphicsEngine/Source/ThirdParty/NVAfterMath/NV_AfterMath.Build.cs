using EngineBuildTool;

class AfterMathBuild : ExternalModuleDef
{
    public AfterMathBuild()
    {
        ModuleRoot = "\\NVAfterMath\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        CommonLibs.Add("GFSDK_Aftermath_Lib.lib");
        DLLs.Add("GFSDK_Aftermath_Lib.dll");
    }
}