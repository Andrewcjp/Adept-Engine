#include "RayTraceReflectionsNode.h"
#include "Rendering/RayTracing/RayTracingEngine.h"
#include "Rendering/RenderNodes/RenderNode.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "RHI/DeviceContext.h"
#include "../../RayTracing/ShaderBindingTable.h"
#include "../../RayTracing/RHIStateObject.h"
#include "../../RayTracing/RayTracingCommandList.h"
#include "../../Core/SceneRenderer.h"
#include "../../Core/LightCulling/LightCullingEngine.h"
#include "../StoreNodes/ShadowAtlasStorageNode.h"
#include "Core/BaseWindow.h"

RayTraceReflectionsNode::RayTraceReflectionsNode()
{
	OnNodeSettingChange();
}


RayTraceReflectionsNode::~RayTraceReflectionsNode()
{}

void RayTraceReflectionsNode::OnExecute()
{
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	FrameBuffer* Gbuffer = GetFrameBufferFromInput(1);

	if (RHI::GetFrameCount() == 0)
	{
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
	RTList->GetRHIList()->SetFrameBufferTexture(Gbuffer, 3, 1);
	RTList->GetRHIList()->SetFrameBufferTexture(Gbuffer, 4, 0);
	SceneRenderer::Get()->BindLightsBuffer(RTList->GetRHIList(), 5);
	SceneRenderer::Get()->GetLightCullingEngine()->GetLightDataBuffer()->BindBufferReadOnly(RTList->GetRHIList(), 6);
	RTList->GetRHIList()->SetConstantBufferView(CBV, 0, 2);

	GetShadowDataFromInput(2)->BindPointArray(RTList->GetRHIList(), 7);


	RTList->SetHighLevelAccelerationStructure(RayTracingEngine::Get()->GetHighLevelStructure());
	RTList->TraceRays(RHIRayDispatchDesc(Target));
	RTList->GetRHIList()->EndTimer(EGPUTIMERS::RT_Trace);
	RTList->Execute();

	RTList->GetRHIList()->GetDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
	PassNodeThough(0, StorageFormats::ScreenReflectionData);
}

bool RayTraceReflectionsNode::IsNodeSupported(const RenderSettings& settings)
{
	if (Context->GetCaps().RTSupport == ERayTracingSupportType::None)
	{
		return false;
	}
	if (!settings.RaytracingEnabled())
	{
		return false;
	}
	return true;
}

std::string RayTraceReflectionsNode::GetName() const
{
	return "RayTraced Reflections";
}

void RayTraceReflectionsNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "OutputBuffer");
	AddInput(EStorageType::Framebuffer, StorageFormats::GBufferData, "GBuffer");
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData, "Shadows");
	AddOutput(EStorageType::Framebuffer, StorageFormats::ScreenReflectionData, "Screen Data");
}

void RayTraceReflectionsNode::OnSetupNode()
{
	RTList = RayTracingEngine::CreateRTList(RHI::GetDefaultDevice());
	DefaultTable = new ShaderBindingTable();

	DefaultTable->InitReflections();

	StateObject = RHI::GetRHIClass()->CreateStateObject(RHI::GetDefaultDevice());
	StateObject->ShaderTable = DefaultTable;
	StateObject->Build();
	RayTracingEngine::Get()->AddHitTable(DefaultTable);
	CBV = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	CBV->CreateConstantBuffer(sizeof(Data), 1);
}
