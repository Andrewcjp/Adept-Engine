#pragma once
class RenderGraphProcessor
{
public:
	RenderGraphProcessor();
	~RenderGraphProcessor();
	static void Process(RenderGraph* graph);
private:
	static void BuildTransitions(RenderGraph * graph);
	static void BuildAliasing(RenderGraph * graph);
};

