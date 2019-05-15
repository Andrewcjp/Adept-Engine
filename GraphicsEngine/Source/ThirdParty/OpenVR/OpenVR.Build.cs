using EngineBuildTool;

class OpenVRBuild : ExternalModuleDef
{
    public OpenVRBuild()
    {
        ModuleRoot = "\\OpenVR\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        CommonLibs.Add("openvr_api.lib");
        DLLs.Add("openvr_api.dll");
    }
}