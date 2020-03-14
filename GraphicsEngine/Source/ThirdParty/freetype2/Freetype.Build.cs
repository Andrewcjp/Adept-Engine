using EngineBuildTool;

class freetypeBuild : ExternalModuleDef
{
    public freetypeBuild()
    {
        ModuleRoot = "\\freetype2\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        LibDirs.Add("../source/ThirdParty/freetype2/");
        StaticLibraries.Add(new LibDependency("freetype.lib", "all"));
        Defines.Add("SUPPORT_FREETYPE");
    }
}