#include "RenderGraphSystem.h"
#include "RenderGraph.h"


RenderGraphSystem::RenderGraphSystem()
{}


RenderGraphSystem::~RenderGraphSystem()
{}

void RenderGraphSystem::Test()
{
	CurrentGraph = new RenderGraph();
	CurrentGraph->CreateTestgraph();
	CurrentGraph->RunGraph();
}
