using EngineBuildTool;

class D3D12RHIModule : ModuleDef
{
    public D3D12RHIModule(TargetRules Rules) : base(Rules)
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
        
        ThirdPartyModules.Add("Dx12Build");
        ThirdPartyModules.Add("OpenVRBuild");
        ThirdPartyModules.Add("PixBuild");
        NuGetPackages.Add("WinPixEventRuntime");
        UnsupportedPlatforms.Add("Win64_VK");
        UnsupportedPlatforms.Add("Linux");
        UnsupportedPlatforms.Add("Android");

        string Win64 = "win64";
        StaticLibraries.Add(new LibDependency("d3d12.lib", Win64));
        StaticLibraries.Add(new LibDependency("dxguid.lib", Win64));
        StaticLibraries.Add(new LibDependency("dxgi.lib", Win64));
        StaticLibraries.Add(new LibDependency("d3dcompiler.lib", Win64));
        if (Rules.Win_SupportsRT())
        {
            StaticLibraries.Add(new LibDependency("dxcompiler.lib", Win64));
        }
    }
}