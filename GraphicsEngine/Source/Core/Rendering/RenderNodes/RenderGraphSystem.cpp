#include "RenderGraphSystem.h"
#include "RenderGraph.h"
#include "Core/Input/Input.h"
#include "RenderNode.h"

RenderGraphSystem::RenderGraphSystem()
{}

RenderGraphSystem::~RenderGraphSystem()
{
	SafeDelete(CurrentGraph);
}

void RenderGraphSystem::InitGraph()
{
	CurrentGraph = new RenderGraph();
	//CurrentGraph->CreateFWDGraph();
	//CurrentGraph->CreateVRFWDGraph();
	//CurrentGraph->CreateDefTestgraph();
	CurrentGraph->CreatePathTracedGraph();
	CurrentGraph->BuildGraph();
}

void RenderGraphSystem::Render()
{
	CurrentGraph->RunGraph();
}

void RenderGraphSystem::Update()
{
	if (Input::GetKeyDown('U'))
	{		
		CurrentGraph->ToggleCondition("PREZ");		
	}
	if (Input::GetKeyDown('Y'))
	{
		CurrentGraph->ToggleCondition("Debug");
	}
#if 0
	if (Input::GetKeyDown('I'))
	{
		RenderGraph* Test = new RenderGraph();
		Test->CreateVRFWDGraph();
		SwitchGraph(Test);
	}
#endif
}

void RenderGraphSystem::SwitchGraph(RenderGraph* NewGraph)
{
	RHI::WaitForGPU();
	SafeDelete(CurrentGraph);
	CurrentGraph = NewGraph;
	CurrentGraph->BuildGraph();
}

RenderGraph * RenderGraphSystem::GetCurrentGraph()
{
	return CurrentGraph;
}
