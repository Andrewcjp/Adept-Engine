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
#include "Core/EngineTypes.h"
#include "../../Core/FrameBuffer.h"
#include "../NodeLink.h"
#include "../../Shaders/Shader_Pair.h"
#include "../../Core/DynamicQualityEngine.h"
#include "RHI/RHIBufferGroup.h"
#include "Rendering/RayTracing/RayTracingScene.h"

RayTraceReflectionsNode::RayTraceReflectionsNode()
{
	OnNodeSettingChange();
	NodeEngineType = ECommandListType::Compute;
}


RayTraceReflectionsNode::~RayTraceReflectionsNode()
{}

void RayTraceReflectionsNode::OnExecute()
{
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	FrameBuffer* Gbuffer = GetFrameBufferFromInput(1);


	//StateObject->RebuildShaderTable();

	FLAT_COMPUTE_START(RTList->GetRHIList()->GetDevice());

	Data.IProj = glm::inverse(SceneRenderer::Get()->GetCurrentCamera()->GetProjection());
	Data.IView = glm::inverse(SceneRenderer::Get()->GetCurrentCamera()->GetView());
	Data.CamPos = SceneRenderer::Get()->GetCurrentCamera()->GetPosition();
	CBV->UpdateConstantBuffer(&Data, 0);

	RTList->ResetList();
	SetBeginStates(RTList->GetRHIList());

	SceneRenderer::Get()->GetRTScene()->UpdateShaderTable(BindingTable);
	RTList->SetStateObject(SceneRenderer::Get()->GetRTScene()->GetSceneStateObject());
	RTList->SetHighLevelAccelerationStructure(SceneRenderer::Get()->GetRTScene()->GetTopLevel());
	RTList->GetRHIList()->SetFrameBufferTexture(Gbuffer, 3, 1);
	RTList->GetRHIList()->SetFrameBufferTexture(Gbuffer, 4, 0);
	RTList->GetRHIList()->SetFrameBufferTexture(Gbuffer, 9, 2);
	SceneRenderer::Get()->BindLightsBufferB(RTList->GetRHIList(), 5);
	SceneRenderer::Get()->GetLightCullingEngine()->GetLightDataBuffer()->Get(RTList->GetRHIList())->BindBufferReadOnly(RTList->GetRHIList(), 6);
	RTList->GetRHIList()->SetConstantBufferView(CBV, 0, 2);
	DynamicQualityEngine::Get()->BindRTBuffer(RTList->GetRHIList(), 10);
	GetShadowDataFromInput(2)->BindPointArray(RTList->GetRHIList(), 7);

	RHIRayDispatchDesc raydesc = RHIRayDispatchDesc(Target);
	raydesc.RayArguments.RayFlags = RAY_FLAGS::RAY_FLAG_FORCE_OPAQUE;
	raydesc.PushRayArgs = true;
	RTList->TraceRays(raydesc);


	SetEndStates(RTList->GetRHIList());
	RTList->Execute();

	FLAT_COMPUTE_END(RTList->GetRHIList()->GetDevice());
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

void RayTraceReflectionsNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::UAV, StorageFormats::DefaultFormat, "OutputBuffer");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::GBufferData, "GBuffer");
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData, "Shadows");	
}

void RayTraceReflectionsNode::OnSetupNode()
{
	RTList = RayTracingEngine::CreateRTList(RHI::GetDefaultDevice());
	BindingTable = new ReflectionsBindingTable();
	BindingTable->InitTable();
	RHIStateObjectDesc Desc = {};
	Desc.AttibuteSize = sizeof(glm::vec2);// float2 barycentrics
	Desc.PayloadSize = sizeof(glm::vec4) * 3;    // float4 pixelColor
	//StateObject = RHI::GetRHIClass()->CreateStateObject(RHI::GetDefaultDevice(), Desc);
	//StateObject->ShaderTable = BindingTable;
	//StateObject->Build();
	//RayTracingEngine::Get()->AddHitTable(BindingTable);
	CBV = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	CBV->CreateConstantBuffer(sizeof(Data), 1);
	
}

void RayTraceReflectionsNode::OnValidateNode(RenderGraph::ValidateArgs & args)
{
	if (Context->GetCaps().RTSupport == ERayTracingSupportType::None)
	{
		args.AddError("RayTraceReflectionsNode on GPU with no ray tracing");
	}
}
