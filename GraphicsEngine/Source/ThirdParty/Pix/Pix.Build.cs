using EngineBuildTool;

class PixBuild : ExternalModuleDef
{
    public PixBuild()
    {
        ModuleRoot = "\\Pix\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        DLLs.Add("WinPixEventRuntime.dll");
        LibDirs.Add("../source/ThirdParty/Pix/");
        StaticLibraries.Add(new LibDependency("WinPixEventRuntime.lib", "win64"));
    }
}