using EngineBuildTool;

class D3D12RHIModule : ModuleDef
{
    public D3D12RHIModule()
    {
        ModuleName = "D3D12RHI";
        SourceFileSearchDir = "D3D12RHI";
        //  ModuleDepends.Add("Core");
        ModuleOuputType = ModuleDef.ModuleType.LIB;
    }
}