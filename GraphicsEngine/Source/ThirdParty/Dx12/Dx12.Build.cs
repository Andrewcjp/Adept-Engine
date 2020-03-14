using EngineBuildTool;

class Dx12Build : ExternalModuleDef
{
    public Dx12Build()
    {
        ModuleRoot = "\\Dx12\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        DLLs.Add("dxil.dll");
    }
}