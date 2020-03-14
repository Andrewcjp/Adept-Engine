using EngineBuildTool;

class WindowsShaderCompilerModule : ModuleDef
{
    public WindowsShaderCompilerModule(TargetRules Rules) : base(Rules)
    {
        SolutionFolderPath = "Engine/ShaderCompilers/Windows";
        ModuleName = "WindowsShaderCompiler";
        SourceFileSearchDir = "WindowsShaderCompiler";
        PCH = "WindowsShaderPCH";
        ModuleOutputType = ModuleDef.ModuleType.DLL;
        UseCorePCH = false;
        UseUnity = true;
        IncludeDirectories.Add("$(Console_SdkIncludeRoot)");
        SystemLibNames.Add("d3d12.lib");
        SystemLibNames.Add("dxcompiler.lib");
        UnsupportedPlatforms.Add("X*");
    }
}