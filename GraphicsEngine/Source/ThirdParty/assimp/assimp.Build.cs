using EngineBuildTool;

class assimpBuild : ExternalModuleDef
{
    public assimpBuild()
    {
        ModuleRoot = "\\assimp\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);        
        CommonLibs.Add("assimp.lib");
        DLLs.Add("assimp-vc140-mt.dll");
    }
}