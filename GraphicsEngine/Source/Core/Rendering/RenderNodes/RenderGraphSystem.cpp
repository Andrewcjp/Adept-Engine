#include "RenderGraphSystem.h"
#include "Core/Input/Input.h"
#include "RenderGraph.h"
#include "RenderGraphPatchLibrary.h"
#include "Rendering/Performance/GPUPerformanceTestManager.h"
#include "Rendering/Renderers/RenderSettings.h"
#include "RenderNode.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHI.h"


RenderGraphSystem::RenderGraphSystem()
{
	//UseRGISSystem = true;
	CurrentSet = new RenderGraphInstanceSet();
	CurrentSet->Init();
	PatchLib = new RenderGraphPatchLibrary();
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
		CurrentGraph = CreateGraph(new RenderGraphInstance(RHI::GetRenderSettings()->SelectedGraph, EBuiltInRenderGraphPatch::NONE));
	}
}

RenderGraph* RenderGraphSystem::CreateGraph(RenderGraphInstance* instance)
{
	RenderGraph* Graph = new RenderGraph();
	instance->Instance = Graph;
#if !BUILD_SHIPPING
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRX())
	{
		instance->Patches.push_back(EBuiltInRenderGraphPatch::VRX);
	}
	if (instance->GraphBaseType == EBuiltinRenderGraphs::DeferredRenderer_RT)
	{
		instance->Patches.push_back(EBuiltInRenderGraphPatch::RT_Reflections);
		instance->GraphBaseType = EBuiltinRenderGraphs::DeferredRenderer;
	}
	if (instance->GraphBaseType == EBuiltinRenderGraphs::DeferredRenderer_VX)
	{
		instance->Patches.push_back(EBuiltInRenderGraphPatch::Voxel_Reflections);
		instance->GraphBaseType = EBuiltinRenderGraphs::DeferredRenderer;
	}
	if (instance->GraphBaseType == EBuiltinRenderGraphs::DeferredRenderer_VX_RT)
	{
		instance->Patches.push_back(EBuiltInRenderGraphPatch::RT_Voxel_Reflections);
		instance->GraphBaseType = EBuiltinRenderGraphs::DeferredRenderer;
	}
#endif
	BuildingGraph = Graph;
	InitDefaultGraph(Graph, instance->GraphBaseType);
	PatchGraph(instance);
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
	case EBuiltinRenderGraphs::DeferredRenderer_VX_RT:
		Graph->CreateDefGraphWithVoxelRT();
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
	case EBuiltinRenderGraphs::MGPU_SFR:
		Graph->CreateSFR();
		break;
	case EBuiltinRenderGraphs::MGPU_ASYNCSHADOWS:
		Graph->CreateMGPUShadows();
		break;
	}
}

void RenderGraphSystem::PatchGraph(RenderGraphInstance* Instance)
{
	for (int i = 0; i < Instance->Patches.size(); i++)
	{
		AD_Assert(PatchLib->ApplyPatch(Instance->Patches[i], Instance->Instance),"Failed to apply patch");
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
	if (Input::GetKeyDown(KeyCode::U))
	{
		CurrentGraph->ToggleCondition("PREZ");
	}
	if (Input::GetKeyDown(KeyCode::Y))
	{
		CurrentGraph->ToggleCondition("Debug");
	}
	if (Input::GetKeyDown(KeyCode::R))
	{
		RHI::GetRenderSettings()->SetVRXActive(!RHI::GetRenderSettings()->GetVRXSettings().VRXActive);
	}
#if 0
	if (Input::GetKeyDown(KeyCode::I))
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
	Inst->Instance = CreateGraph(Inst);
	return Inst;
}

RenderGraph * RenderGraphSystem::GetCurrentGraph()
{
	return CurrentGraph;
}

RenderGraph* RenderGraphSystem::GetGraphBuilding()
{
	return BuildingGraph;
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

