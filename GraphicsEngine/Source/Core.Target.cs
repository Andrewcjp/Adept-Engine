
using EngineBuildTool;

class CoreTargetRules : TargetRules
{
    bool BuildPhysx = true;
    bool BuildVulkan = true;
    public CoreTargetRules()
    {
        if (!BuildVulkan)
        {
            ModuleExcludeList.Add("VulkanRHI");
        }

        if (BuildPhysx)
        {
            GlobalDefines.Add("USE_PHYSX");
            ModuleExcludeList.Add("TDPhysics");
        }
        WindowTenVersionTarget = "1903";
       // WindowTenVersionTarget = "1803";
    }
    public override ModuleDef GetCoreModule()
    {
        ModuleDef CoreModule = new ModuleDef(this);
        CoreModule.ModuleName = "Core";
        CoreModule.ModuleOutputType = ModuleDef.ModuleType.DLL;
        CoreModule.SolutionFolderPath = "Engine/Core";
        CoreModule.SourceFileSearchDir = "Core";
        CoreModule.PCH = "Stdafx";
        CoreModule.IncludeDirectories.Add("/source/ThirdParty");
        CoreModule.IncludeDirectories.Add("/source/Core");
        CoreModule.IncludeDirectories.Add("/source/ThirdParty/freetype2");
        CoreModule.IncludeDirectories.Add("/source/TDPhysics");
        CoreModule.UseCorePCH = false;
        
        CoreModule.UseUnity = true;
        CoreModule.OutputObjectName = "Core";

        CoreModule.IsCoreModule = true;
        CoreModule.SystemLibNames.Add("winmm.lib");
        CoreModule.SystemLibNames.Add("shlwapi.lib");
        CoreModule.SystemLibNames.Add("dbghelp.lib");
        string[] ThirdPartyModules = new string[] {
            "freetypeBuild",
            "assimpBuild",
            "nvapiBuild",
            "WWiseBuild",
            "OpenVRBuild",
            "AfterMathBuild"
        };
        CoreModule.ThirdPartyModules.AddRange(ThirdPartyModules);
        if (BuildPhysx)
        {
            CoreModule.ThirdPartyModules.Add("PhysxBuild");
        }
        else
        {
            CoreModule.ModuleDepends.Add("TDPhysics");
        }
        if (BuildVulkan)
        {
            CoreModule.PreProcessorDefines.Add("WITH_VK");
        }
        return CoreModule;
    }
}