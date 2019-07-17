#include "PathTraceSceneNode.h"
#include "..\..\RayTracing\RayTracingEngine.h"
#include "..\StorageNodeFormats.h"
#include "Core\BaseWindow.h"
#include "..\..\RayTracing\RayTracingCommandList.h"
#include "..\..\RayTracing\RHIStateObject.h"
#include "..\..\RayTracing\ShaderBindingTable.h"


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
	Data.IProj = glm::inverse(BaseWindow::GetCurrentCamera()->GetProjection());
	Data.IView = glm::inverse(BaseWindow::GetCurrentCamera()->GetView());
	Data.CamPos = BaseWindow::GetCurrentCamera()->GetPosition();
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

std::string PathTraceSceneNode::GetName() const
{
	return "Path Trace";
}

void PathTraceSceneNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat);
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
}

void PathTraceSceneNode::OnSetupNode()
{
	RTList = RayTracingEngine::CreateRTList(RHI::GetDefaultDevice());
	DefaultTable = new ShaderBindingTable();

	DefaultTable->InitDefault();

	StateObject = RHI::GetRHIClass()->CreateStateObject(RHI::GetDefaultDevice());
	StateObject->ShaderTable = DefaultTable;
	StateObject->Build();
	RayTracingEngine::Get()->AddHitTable(DefaultTable);
	CBV = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	CBV->CreateConstantBuffer(sizeof(Data), 1);
}
