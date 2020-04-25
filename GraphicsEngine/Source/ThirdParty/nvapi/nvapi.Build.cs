using EngineBuildTool;
using System.IO;
class nvapiBuild : ExternalModuleDef
{
    public nvapiBuild()
    {
        ModuleRoot = "\\nvapi\\";
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, false);
        AddLibSearch(ref LibrarySearchPaths, "", LibBuildConfig.General, true);
        if (File.Exists(ModuleDefManager.GetRootPath()+"\\Source\\ThirdParty\\nvapi\\Include\\nvapi.h"))
        {
            StaticLibraries.Add(new LibDependency("nvapi64.lib", "win64"));
            UnsupportedPlatforms.Add("X*");
            LibDirs.Add("../source/ThirdParty/nvapi/");
        }
    }
}