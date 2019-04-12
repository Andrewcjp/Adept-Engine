using EngineBuildTool;

class VulkanBuild : ExternalModuleDef
{
    public VulkanBuild()
    {
        ModuleRoot = "\\vulkan\\";
        AddStandardFolders(false);
    }
}