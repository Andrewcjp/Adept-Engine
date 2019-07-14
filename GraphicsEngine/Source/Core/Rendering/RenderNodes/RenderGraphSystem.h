#pragma once

class RenderGraph;
class RenderGraphSystem
{
public:
	RenderGraphSystem();
	~RenderGraphSystem();
	void InitGraph();
	void Render();
	void Update();
	void SwitchGraph(RenderGraph * NewGraph);
	RenderGraph* GetCurrentGraph();
private:
	RenderGraph* CurrentGraph = nullptr;
};

