using EngineBuildTool;

class VulkanRHIModule : ModuleDef
{
    public VulkanRHIModule(TargetRules Rules) : base(Rules)
    {
        SolutionFolderPath = "Engine/Modules/RHI";
        ModuleName = "VulkanRHI";
        SourceFileSearchDir = "VulkanRHI";
        ModuleOutputType = ModuleDef.ModuleType.ModuleDLL;
        UseCorePCH = false;
        PCH = "VulkanRHIPCH";
        UseUnity = true;
        PreProcessorDefines.Add("WITH_VK");
        ThirdPartyModules.Add("VulkanBuild");
        UnsupportedPlatforms.Add("Win64_DX12");
    }
}