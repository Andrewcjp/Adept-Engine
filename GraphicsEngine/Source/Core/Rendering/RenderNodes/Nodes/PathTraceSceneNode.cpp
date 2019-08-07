#include "PathTraceSceneNode.h"
#include "Core/BaseWindow.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/RayTracing/RayTracingCommandList.h"
#include "Rendering/RayTracing/RayTracingEngine.h"
#include "Rendering/RayTracing/RHIStateObject.h"
#include "Rendering/RayTracing/ShaderBindingTable.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../../RayTracing/Tables/PathTraceBindingTable.h"


PathTraceSceneNode::PathTraceSceneNode()
{
	OnNodeSettingChange();
}


PathTraceSceneNode::~PathTraceSceneNode()
{}

void PathTraceSceneNode::OnExecute()
{
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	if (RHI::GetFrameCount() == 0)
	{
		RHI::WaitForGPU();
		StateObject->RebuildShaderTable();
	}
	RTList->GetRHIList()->GetDevice()->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
	Data.IProj = glm::inverse(SceneRenderer::Get()->GetCurrentCamera()->GetProjection());
	Data.IView = glm::inverse(SceneRenderer::Get()->GetCurrentCamera()->GetView());
	Data.CamPos = SceneRenderer::Get()->GetCurrentCamera()->GetPosition();
	CBV->UpdateConstantBuffer(&Data, 0);

	RTList->ResetList();

	RTList->GetRHIList()->StartTimer(EGPUTIMERS::RT_Trace);
	RTList->SetStateObject(StateObject);
	//Target->MakeReadyForComputeUse(RTList->GetRHIList());
	RTList->SetHighLevelAccelerationStructure(RayTracingEngine::Get()->GetHighLevelStructure());
	RTList->GetRHIList()->SetConstantBufferView(CBV, 0, 2);
	RTList->TraceRays(RHIRayDispatchDesc(Target));
	RTList->GetRHIList()->EndTimer(EGPUTIMERS::RT_Trace);
	RTList->Execute();

	RTList->GetRHIList()->GetDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
	PassNodeThough(0, StorageFormats::LitScene);
}


void PathTraceSceneNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat);
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
}

void PathTraceSceneNode::OnSetupNode()
{
	RTList = RayTracingEngine::CreateRTList(RHI::GetDefaultDevice());
	DefaultTable = new PathTraceBindingTable();

	DefaultTable->InitTable();

	StateObject = RHI::GetRHIClass()->CreateStateObject(RHI::GetDefaultDevice());
	StateObject->ShaderTable = DefaultTable;
	StateObject->Build();
	RayTracingEngine::Get()->AddHitTable(DefaultTable);
	CBV = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	CBV->CreateConstantBuffer(sizeof(Data), 1);
}
