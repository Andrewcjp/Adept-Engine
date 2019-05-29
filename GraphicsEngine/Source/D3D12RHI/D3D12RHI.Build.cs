using EngineBuildTool;

class D3D12RHIModule : ModuleDef
{
    public D3D12RHIModule()
    {
        ModuleName = "D3D12RHI";
        SourceFileSearchDir = "D3D12RHI";
        ModuleOutputType = ModuleDef.ModuleType.LIB;
        IncludeDirectories.Add("Source/Core");       
        SolutionFolderPath = "Engine/Modules/RHI";
        UseCorePCH = false;
        PCH = "D3D12RHIPCH";
        ModuleOutputType = ModuleDef.ModuleType.ModuleDLL;
        UseUnity = true;
        SystemLibNames.Add("d3d12.lib");
        SystemLibNames.Add("dxguid.lib");
        SystemLibNames.Add("dxgi.lib");
        SystemLibNames.Add("d3dcompiler.lib");
        SystemLibNames.Add("dxcompiler.lib");
        ThirdPartyModules.Add("Dx12Build");
    }
}