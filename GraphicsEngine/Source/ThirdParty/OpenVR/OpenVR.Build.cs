using EngineBuildTool;

class OpenVRBuild : ExternalModuleDef
{
    public OpenVRBuild()
    {
        ModuleRoot = "\\OpenVR\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        DLLs.Add("openvr_api.dll");
        UnsupportedPlatforms.Add("X*");
        Defines.Add("SUPPORT_OPENVR");
        StaticLibraries.Add(new LibDependency("openvr_api.lib", "win64"));
        LibDirs.Add("../source/ThirdParty/OpenVR/");
    }
}