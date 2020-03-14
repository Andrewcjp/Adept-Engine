using EngineBuildTool;

class assimpBuild : ExternalModuleDef
{
    public assimpBuild()
    {
        ModuleRoot = "\\assimp\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        StaticLibraries.Add(new LibDependency("assimp.lib", "win64"));
        DLLs.Add("assimp-vc140-mt.dll");
        UnsupportedPlatforms.Add("X*");
        Defines.Add("BUILD_ASSIMP");
        LibDirs.Add("../source/ThirdParty/assimp/");
    }
}