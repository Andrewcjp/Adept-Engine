#pragma once
namespace EBuiltinRenderGraphs
{
	enum Type
	{
		Fallback,//in the event of error this graph is used
		DeferredRenderer,
		DeferredRenderer_RT,
		DeferredRenderer_VX,
		DeferredRenderer_VX_RT,
		ForwardRenderer,
		VRForwardRenderer,
		Pathtracing,
		TEST_MGPU,
		MGPU_SFR,
		MGPU_ASYNCSHADOWS,
		Custom,
		Limit
	};
}
namespace EBuiltInRenderGraphPatch
{
	enum Type
	{
		NONE,
		//MGPU
		MainFramebufferSFR,
		PostProccessOnSecondGPU,
		//MGPU shadows
		MGPU_ShadowMapping,
		Async_MGPU_ShadowMapping,
		//VR
		VR_GPUPerEye,//single GPU per eye (balanced only)
		VR_GPUSFRPerEye,//SFR is used across both eyes as if one big RT(Unbalanced)
		//RT
		RT_Reflections,
		RT_Shadows,
		RT_Transparency,
		RT_Realtime_GI,
		//VRR
		VRX,

		Voxel_Reflections,
		Voxel_GI,

		RT_Voxel_Reflections,

		Custom,
		Limit,
	};
}
class RenderGraph;
struct RenderSettings;
class RenderGraphPatchLibrary;
//defines a created graph instance.
struct RenderGraphInstance
{
	RenderGraphInstance() {}
	RenderGraphInstance(EBuiltinRenderGraphs::Type base, EBuiltInRenderGraphPatch::Type patch = EBuiltInRenderGraphPatch::NONE)
	{
		GraphBaseType = base;
		Patches.push_back(patch);
	}
	std::vector<EBuiltInRenderGraphPatch::Type> Patches;
	EBuiltinRenderGraphs::Type GraphBaseType = EBuiltinRenderGraphs::DeferredRenderer;
	RenderGraph* Instance = nullptr;
};
//this object determines what instances should be used.
//override this to change how the engine picks graph 
struct RenderGraphInstanceSet
{
	RenderGraphInstance* GetInstanceForSettings(RenderSettings* set);

	RenderGraphInstance* HDGraph = nullptr;
	RenderGraphInstance* LowSpecGraph = nullptr;

	RenderGraphInstance* RTGraph = nullptr;
	RenderGraphInstance* MGPUGraph = nullptr;
	//VR
	RenderGraphInstance* VrGraph = nullptr;
	RenderGraphInstance* VrMGPUGraph = nullptr;
	void Init();
};
class RenderGraphSystem
{
public:
	RenderGraphSystem();
	~RenderGraphSystem();
	void InitGraph();



	void CheckGraph(RenderGraph* Graph);

	void InitDefaultGraph(RenderGraph* Graph, EBuiltinRenderGraphs::Type  SelectedGraph);
	void Render();
	void Update();
	void SwitchGraph(RenderGraph * NewGraph);

	RenderGraphInstance* BuildInstance(RenderGraphInstance* Inst);

	RenderGraph* GetCurrentGraph();
	RenderGraph* GetGraphBuilding();
	void PatchGraph(RenderGraphInstance* Graph);
	bool UseRGISSystem = false;
private:
	RenderGraphInstanceSet* CurrentSet = nullptr;
	RenderGraph* CurrentGraph = nullptr;
	RenderGraph* BuildingGraph = nullptr;
	RenderGraphInstance* CurrentInstance = nullptr;
	RenderGraph* CreateGraph(RenderGraphInstance* instance);

	RenderGraphPatchLibrary* PatchLib = nullptr;
};

