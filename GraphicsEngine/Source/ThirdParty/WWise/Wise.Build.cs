using EngineBuildTool;

class WWiseBuild : ExternalModuleDef
{
    public WWiseBuild()
    {
        ModuleRoot = "\\WWise\\";
        AddStandardFolders(false);
    }
}