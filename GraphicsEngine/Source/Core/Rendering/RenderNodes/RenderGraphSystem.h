#pragma once
namespace EBuiltinRenderGraphs
{
	enum Type
	{
		Fallback,//in the event of error this graph is used
		DeferredRenderer,
		DeferredRenderer_RT,
		ForwardRenderer,
		VRForwardRenderer,
		Pathtracing,
		TEST_MGPU,
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

		Custom,
		Limit,
	};
}
class RenderGraph;
struct RenderSettings;
//defines a created graph instance.
struct RenderGraphInstance
{
	RenderGraphInstance() {}
	RenderGraphInstance(EBuiltinRenderGraphs::Type base, EBuiltInRenderGraphPatch::Type patch = EBuiltInRenderGraphPatch::NONE)
	{
		GraphBaseType = base;
		Patch = patch;
	}
	EBuiltInRenderGraphPatch::Type Patch = EBuiltInRenderGraphPatch::NONE;
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
	void PatchGraph(RenderGraph* Graph, EBuiltInRenderGraphPatch::Type patch);
	bool UseRGISSystem = false;
private:
	RenderGraphInstanceSet* CurrentSet = nullptr;
	RenderGraph* CurrentGraph = nullptr;
	RenderGraphInstance* CurrentInstance = nullptr;
	RenderGraph* CreateGraph(EBuiltinRenderGraphs::Type GraphBaseType, EBuiltInRenderGraphPatch::Type Patch);
};

