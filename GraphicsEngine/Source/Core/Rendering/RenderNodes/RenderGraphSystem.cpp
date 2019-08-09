#include "RenderGraphSystem.h"
#include "RenderGraph.h"
#include "Core/Input/Input.h"
#include "RenderNode.h"
#include "../Renderers/RenderSettings.h"

RenderGraphSystem::RenderGraphSystem()
{}

RenderGraphSystem::~RenderGraphSystem()
{
	SafeDelete(CurrentGraph);
}

void RenderGraphSystem::InitGraph()
{
	CurrentGraph = new RenderGraph();
	InitDefaultGraph();
	PatchGraph();
	CheckGraph();

	CurrentGraph->BuildGraph();
	CurrentGraph->RunTests();
}

void RenderGraphSystem::CheckGraph()
{
	if (!CurrentGraph->IsGraphValid())
	{
		CurrentGraph->DestoryGraph();
		std::string Data = "Selected graph '" + CurrentGraph->GetGraphName() + "' is invalid. Engine will use the fallback graph";
		Log::LogMessage(Data, Log::Error);
		PlatformApplication::DisplayMessageBox("Error", Data);
		CurrentGraph->CreateFallbackGraph();
	}
}

void RenderGraphSystem::InitDefaultGraph()
{
	switch (RHI::GetRenderSettings()->SelectedGraph)
	{
		case EBuiltinRenderGraphs::Fallback:
			CurrentGraph->CreateFallbackGraph();
			break;
		case EBuiltinRenderGraphs::ForwardRenderer:
			CurrentGraph->CreateFWDGraph();
			break;
		case EBuiltinRenderGraphs::DeferredRenderer:
			CurrentGraph->CreateDefTestgraph();
			break;
		case EBuiltinRenderGraphs::DeferredRenderer_RT:
			CurrentGraph->CreateDefGraphWithRT();
			break;
		case EBuiltinRenderGraphs::VRForwardRenderer:
			CurrentGraph->CreateVRFWDGraph();
			break;
		case EBuiltinRenderGraphs::Pathtracing:
			CurrentGraph->CreatePathTracedGraph();
			break;
		case EBuiltinRenderGraphs::TEST_MGPU:
			CurrentGraph->CreateMGPU_TESTGRAPH();
			break;
	}
}

void RenderGraphSystem::PatchGraph()
{
	switch (RHI::GetRenderSettings()->SelectedPatch)
	{
		case EBuiltInRenderGraphPatch::NONE:
			return;
		case EBuiltInRenderGraphPatch::MainFramebufferSFR:
			break;
		case EBuiltInRenderGraphPatch::PostProccessOnSecondGPU:
			break;
		case EBuiltInRenderGraphPatch::MGPU_ShadowMapping:
			break;
		case EBuiltInRenderGraphPatch::Async_MGPU_ShadowMapping:
			break;
		case EBuiltInRenderGraphPatch::VR_GPUPerEye:
			break;
		case EBuiltInRenderGraphPatch::VR_GPUSFRPerEye:
			break;
	}
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

