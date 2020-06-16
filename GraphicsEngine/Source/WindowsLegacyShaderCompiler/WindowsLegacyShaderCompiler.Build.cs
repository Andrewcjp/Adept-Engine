using EngineBuildTool;

class WindowsLegacyShaderCompilerModule : ModuleDef
{
    public WindowsLegacyShaderCompilerModule(TargetRules Rules) : base(Rules)
    {
        SolutionFolderPath = "Engine/ShaderCompilers/Windows";
        ModuleName = "WindowsLegacyShaderCompiler";
        SourceFileSearchDir = "WindowsLegacyShaderCompiler";
        PCH = "WindowsLegacyShaderCompilerPCH";
        ModuleOutputType = ModuleDef.ModuleType.DLL;
        UseCorePCH = false;
        UseUnity = true;
        SystemLibNames.Add("d3d12.lib");
        SystemLibNames.Add("d3dcompiler.lib");
        UnsupportedPlatforms.Add("X*");
    }
}