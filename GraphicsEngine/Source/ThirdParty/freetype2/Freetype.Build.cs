using EngineBuildTool;

class freetypeBuild : ExternalModuleDef
{
    public freetypeBuild()
    {
        ModuleRoot = "\\freetype2\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        CommonLibs.Add("freetype.lib");
        DLLs.Add("freetype271.dll");
    }
}