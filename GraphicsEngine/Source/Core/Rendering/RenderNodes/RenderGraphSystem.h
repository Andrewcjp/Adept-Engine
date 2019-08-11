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
		MainFramebufferSFR,
		PostProccessOnSecondGPU,
		//shadows
		MGPU_ShadowMapping,
		Async_MGPU_ShadowMapping,
		//VR
		VR_GPUPerEye,
		VR_GPUSFRPerEye,

		Custom,
		Limit,
	};
}
class RenderGraph;
//defines a created graph instance.
struct RenderGraphInstance
{	
	EBuiltInRenderGraphPatch::Type Patch = EBuiltInRenderGraphPatch::NONE;
	EBuiltinRenderGraphs::Type GraphBaseType = EBuiltinRenderGraphs::DeferredRenderer;
	RenderGraph* Instance = nullptr;
};

class RenderGraphSystem
{
public:
	RenderGraphSystem();
	~RenderGraphSystem();
	void InitGraph();

	void CheckGraph();

	void InitDefaultGraph();
	void Render();
	void Update();
	void SwitchGraph(RenderGraph * NewGraph);
	RenderGraph* GetCurrentGraph();
	void PatchGraph();
private:
	RenderGraph* CurrentGraph = nullptr;
};

