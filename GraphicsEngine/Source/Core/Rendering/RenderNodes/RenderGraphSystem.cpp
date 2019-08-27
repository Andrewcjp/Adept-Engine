#include "RenderGraphSystem.h"
#include "RenderGraph.h"
#include "Core/Input/Input.h"
#include "RenderNode.h"
#include "../Renderers/RenderSettings.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHI.h"
#include "../Performance/GPUPerformanceTestManager.h"

RenderGraphSystem::RenderGraphSystem()
{
	//UseRGISSystem = true;
	CurrentSet = new RenderGraphInstanceSet();
	CurrentSet->Init();
}

RenderGraphSystem::~RenderGraphSystem()
{
	SafeDelete(CurrentGraph);
}

void RenderGraphSystem::InitGraph()
{
	if (UseRGISSystem)
	{
		CurrentInstance = BuildInstance(CurrentSet->GetInstanceForSettings(RHI::GetRenderSettings()));
		CurrentGraph = CurrentInstance->Instance;
	}
	else
	{
		//during dev this is easier to use.
		CurrentGraph = CreateGraph(RHI::GetRenderSettings()->SelectedGraph, EBuiltInRenderGraphPatch::NONE);
	}
}

RenderGraph* RenderGraphSystem::CreateGraph(EBuiltinRenderGraphs::Type GraphBaseType, EBuiltInRenderGraphPatch::Type Patch)
{
	RenderGraph* Graph = new RenderGraph();
	InitDefaultGraph(Graph, GraphBaseType);
	PatchGraph(Graph, Patch);
	CheckGraph(Graph);

	Graph->BuildGraph();
	Graph->RunTests();
	return Graph;
}

void RenderGraphSystem::CheckGraph(RenderGraph* Graph)
{
	if (!Graph->IsGraphValid())
	{
		Graph->DestoryGraph();
		std::string Data = "Selected graph '" + Graph->GetGraphName() + "' is invalid. Engine will use the fallback graph";
		Log::LogMessage(Data, Log::Error);
		PlatformApplication::DisplayMessageBox("Error", Data);
		Graph->CreateFallbackGraph();
	}
}

void RenderGraphSystem::InitDefaultGraph(RenderGraph* Graph, EBuiltinRenderGraphs::Type SelectedGraph)
{
	switch (SelectedGraph)
	{
		case EBuiltinRenderGraphs::Fallback:
			Graph->CreateFallbackGraph();
			break;
		case EBuiltinRenderGraphs::ForwardRenderer:
			Graph->CreateFWDGraph();
			break;
		case EBuiltinRenderGraphs::DeferredRenderer:
			Graph->CreateDefTestgraph();
			break;
		case EBuiltinRenderGraphs::DeferredRenderer_RT:
			Graph->CreateDefGraphWithRT();
			break;
		case EBuiltinRenderGraphs::VRForwardRenderer:
			Graph->CreateVRFWDGraph();
			break;
		case EBuiltinRenderGraphs::Pathtracing:
			Graph->CreatePathTracedGraph();
			break;
		case EBuiltinRenderGraphs::TEST_MGPU:
			Graph->CreateMGPU_TESTGRAPH();
			break;
	}
}

void RenderGraphSystem::PatchGraph(RenderGraph* Graph, EBuiltInRenderGraphPatch::Type patch)
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
	const int frmae = 1; 
	if (RHI::GetFrameCount() < frmae + 3)
	{
		RHI::RunGPUTests();
	}
	else
	{
		//RHI::RunGPUTests();
	}
	if (RHI::GetFrameCount() == frmae)
	{
		//RHI::RunGPUTests();
	}
	else if (RHI::GetFrameCount() == frmae + 3)
	{
		RHI::Get()->GetTestManager()->GatherResults();
	}
	//else if (RHI::GetFrameCount() == frmae + 10)
	//{
	//	RHI::Get()->GetTestManager()->DestoryTests();
	//}
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

RenderGraphInstance* RenderGraphSystem::BuildInstance(RenderGraphInstance* Inst)
{
	Inst->Instance = CreateGraph(Inst->GraphBaseType, Inst->Patch);
	return Inst;
}

RenderGraph * RenderGraphSystem::GetCurrentGraph()
{
	return CurrentGraph;
}

RenderGraphInstance * RenderGraphInstanceSet::GetInstanceForSettings(RenderSettings * set)
{
	CapabilityData MaxCaps = CapabilityData();
	RHI::GetRenderSettings()->MaxSupportedCaps(MaxCaps);
	if (set->GetRTSettings().Enabled && MaxCaps.RTSupport >= ERayTracingSupportType::DriverBased)
	{
		return RTGraph;
	}
	return HDGraph;
}

void RenderGraphInstanceSet::Init()
{
	RTGraph = new RenderGraphInstance(EBuiltinRenderGraphs::DeferredRenderer_RT);
	HDGraph = new RenderGraphInstance(EBuiltinRenderGraphs::DeferredRenderer);
}

