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

void RenderGraphSystem::Test()
{
	CurrentGraph = new RenderGraph();
	//CurrentGraph->CreateFWDGraph();
	CurrentGraph->CreateDefTestgraph();
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
		CurrentGraph->OptionNode->SetNodeActive(!CurrentGraph->OptionNode->IsNodeActive());
	}
	if (Input::GetKeyDown('Y'))
	{
		CurrentGraph->SetCondition(0, !CurrentGraph->GetCondition(0));
	}

}