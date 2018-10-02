using EngineBuildTool;

class VulkanRHIModule : ModuleDef
{
    public VulkanRHIModule()
    {
        SolutionFolderPath = "Engine/Modules/RHI";
        ModuleName = "VulkanRHI";
        SourceFileSearchDir = "VulkanRHI";
        ModuleOutputType = ModuleDef.ModuleType.ModuleDLL;
    }
}