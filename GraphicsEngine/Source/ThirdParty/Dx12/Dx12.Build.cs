using EngineBuildTool;

class Dx12Build : ExternalModuleDef
{
    public Dx12Build()
    {
        ModuleRoot = "\\Dx12\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        DLLs.Add("d3dcompiler_47.dll");
        DLLs.Add("d3dcsx_47.dll");
        DLLs.Add("dxcompiler.dll");
        DLLs.Add("dxil.dll");
    }
}