using EngineBuildTool;

class WWiseBuild : ExternalModuleDef
{
    public WWiseBuild()
    {
        ModuleRoot = "\\WWise\\";
        AddStandardFolders(false);
        SystemLibNames.Add("dxguid.lib");
        SystemLibNames.Add("ws2_32.lib");
    }
}