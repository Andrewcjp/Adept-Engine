using EngineBuildTool;

class VulkanRHIModule : ModuleDef
{
    public VulkanRHIModule()
    {
        SolutionFolderPath = "Engine/Modules/RHI";
        ModuleName = "VulkanRHI";
        SourceFileSearchDir = "VulkanRHI";
        ModuleDepends.Add("Core");
        ModuleOutputType = ModuleDef.ModuleType.DLL;
    }
}