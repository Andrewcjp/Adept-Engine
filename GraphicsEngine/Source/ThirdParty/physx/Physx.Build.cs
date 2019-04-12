using EngineBuildTool;

class PhysxBuild : ExternalModuleDef
{
    public PhysxBuild()
    {
        ModuleRoot = "\\Physx\\";
        AddStandardFolders();
    }
}