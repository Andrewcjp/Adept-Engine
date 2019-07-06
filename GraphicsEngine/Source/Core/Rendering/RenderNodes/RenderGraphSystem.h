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
private:
	RenderGraph* CurrentGraph = nullptr;
};

