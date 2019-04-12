using EngineBuildTool;

class nvapiBuild : ExternalModuleDef
{
    public nvapiBuild()
    {
        ModuleRoot = "\\nvapi\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        CommonLibs.Add("nvapi64.lib");
        DLLs.Add("nvToolsExt64_1.dll");
    }
}