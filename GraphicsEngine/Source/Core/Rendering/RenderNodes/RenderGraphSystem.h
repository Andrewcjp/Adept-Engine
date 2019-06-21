#pragma once

class RenderGraph;
class RenderGraphSystem
{
public:
	RenderGraphSystem();
	~RenderGraphSystem();
	void Test();
	void Render();
private:
	RenderGraph* CurrentGraph = nullptr;
};

