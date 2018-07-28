using EngineBuildTool;

class VulkanRHIModule : ModuleDef
{
    public VulkanRHIModule()
    {
        ModuleName = "VulkanRHI";
        SourceFileSearchDir = "VulkanRHI";
        //  ModuleDepends.Add("Core");
        ModuleOuputType = ModuleDef.ModuleType.LIB;
    }
}