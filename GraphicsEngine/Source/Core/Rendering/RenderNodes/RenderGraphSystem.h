#pragma once
namespace EBuiltinRenderGraphs
{
	enum Type
	{
		Fallback,//in the event of error this graph is used
		DeferredRenderer,
		ForwardRenderer,
		VRForwardRenderer,
		Pathtracing,
		Custom,
		Limit
	};
}
class RenderGraph;
class RenderGraphSystem
{
public:
	RenderGraphSystem();
	~RenderGraphSystem();
	void InitGraph();
	void InitDefaultGraph();
	void Render();
	void Update();
	void SwitchGraph(RenderGraph * NewGraph);
	RenderGraph* GetCurrentGraph();
private:
	RenderGraph* CurrentGraph = nullptr;
};

