#include "RenderGraphSystem.h"
#include "RenderGraph.h"

RenderGraphSystem::RenderGraphSystem()
{}

RenderGraphSystem::~RenderGraphSystem()
{}

void RenderGraphSystem::Test()
{
	CurrentGraph = new RenderGraph();
	CurrentGraph->CreateFWDGraph();
	//CurrentGraph->CreateDefTestgraph();
	CurrentGraph->BuildGraph();
}

void RenderGraphSystem::Render()
{
	CurrentGraph->RunGraph();
}
