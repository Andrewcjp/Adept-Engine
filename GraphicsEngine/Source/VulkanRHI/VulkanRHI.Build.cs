using EngineBuildTool;

class VulkanRHIModule : ModuleDef
{
    public VulkanRHIModule()
    {
        SolutionFolderPath = "Engine/Modules/RHI";
        ModuleName = "VulkanRHI";
        SourceFileSearchDir = "VulkanRHI";
        ModuleOutputType = ModuleDef.ModuleType.ModuleDLL;
        UseCorePCH = false;
        PCH = "VulkanRHIPCH";
        UseUnity = true;
        AdditonalLibSearchPaths.Add(new LibSearchPath("\\Vulkan\\Release", LibBuildConfig.Optimized));
        AdditonalLibSearchPaths.Add(new LibSearchPath("\\Vulkan\\Debug", LibBuildConfig.Debug));
        PreProcessorDefines.Add("WITH_VK");
    }
}