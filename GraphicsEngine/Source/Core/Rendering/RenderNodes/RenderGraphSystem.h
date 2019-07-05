#pragma once

class RenderGraph;
class RenderGraphSystem
{
public:
	RenderGraphSystem();
	~RenderGraphSystem();
	void Test();
	void Render();
	void Update();
private:
	RenderGraph* CurrentGraph = nullptr;
};

