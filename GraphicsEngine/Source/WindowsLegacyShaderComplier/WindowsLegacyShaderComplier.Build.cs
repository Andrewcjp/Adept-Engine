using EngineBuildTool;

class WindowsLegacyShaderComplierModule : ModuleDef
{
    public WindowsLegacyShaderComplierModule(TargetRules Rules) : base(Rules)
    {
        SolutionFolderPath = "Engine/ShaderCompilers/Windows";
        ModuleName = "WindowsLegacyShaderComplier";
        SourceFileSearchDir = "WindowsLegacyShaderComplier";
        PCH = "WindowsLegacyShaderComplierPCH";
        ModuleOutputType = ModuleDef.ModuleType.DLL;
        UseCorePCH = false;
        UseUnity = true;
        SystemLibNames.Add("d3d12.lib");
        SystemLibNames.Add("d3dcompiler.lib");
        UnsupportedPlatforms.Add("X*");
    }
}