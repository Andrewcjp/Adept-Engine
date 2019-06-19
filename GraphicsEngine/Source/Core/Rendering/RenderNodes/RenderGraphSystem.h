#pragma once

class RenderGraph;
class RenderGraphSystem
{
public:
	RenderGraphSystem();
	~RenderGraphSystem();
	void Test();
private:
	RenderGraph* CurrentGraph = nullptr;
};

