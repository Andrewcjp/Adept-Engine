#include "VoxelReflectionsNode.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_Pair.h"
#include "../../Core/FrameBuffer.h"
#include "../../RayTracing/Voxel/VoxelTracingEngine.h"
#include "../../Core/SceneRenderer.h"
#include "../../Core/Screen.h"
#include "../../Core/Camera.h"
#include "RHI/RHITimeManager.h"
#include "../../Core/LightCulling/LightCullingEngine.h"
#include "RHI/RHICommandList.h"
#include "Core/Assets/Scene.h"
#include "../../Core/DynamicQualityEngine.h"
#include "../StoreNodes/ShadowAtlasStorageNode.h"
#include "RHI/RHIBufferGroup.h"
#include "../../RayTracing/Voxel/VoxelScene.h"
#include "../../RayTracing/Voxel/VoxelTopLevelAccelerationStructure.h"

VoxelReflectionsNode::VoxelReflectionsNode()
{
	NodeEngineType = ECommandListType::Compute;
	OnNodeSettingChange();
}

void VoxelReflectionsNode::OnExecute()
{

	FrameBuffer* Target = GetFrameBufferFromInput(0);
	FrameBuffer* Gbuffer = GetFrameBufferFromInput(1);
	FLAT_COMPUTE_START(List->GetDevice());
	List->ResetList();
	SetBeginStates(List);
	VoxelScene* VXS = SceneRenderer::Get()->GetVoxelScene();
	VXS->UpdateTopLevel(List);
	if (RHI::GetRenderSettings()->GetVoxelSet().Enabled)
	{

		if (RHI::GetRenderSettings()->GetVoxelSet().UseConeTracing)
		{
			ExecuteVoxelConeTracing(Target, Gbuffer);
		}
		else
		{
			ExecuteVoxelTracing(Target, Gbuffer);
		}
	}
	SetEndStates(List);
	List->Execute();
	FLAT_COMPUTE_END(List->GetDevice());
}

void VoxelReflectionsNode::ExecuteVoxelTracing(FrameBuffer* Target, FrameBuffer* Gbuffer)
{
	DECALRE_SCOPEDGPUCOUNTER(List, "Voxel Reflections");
	DataSet.CameraPos = SceneRenderer::Get()->GetCurrentCamera()->GetPosition();
	DataSet.res = glm::ivec2(Target->GetWidth(), Target->GetHeight());
	DataSet.FrameCount = RHI::GetFrameCount();
	CBV->UpdateConstantBuffer(&DataSet);
	RHIPipeLineStateDesc PsoDesc = RHIPipeLineStateDesc::CreateDefault(Traceshader);
	List->SetPipelineStateDesc(PsoDesc);
	List->SetFrameBufferTexture(Gbuffer, "GBUFFER_Pos", 0);
	List->SetFrameBufferTexture(Gbuffer, "GBUFFER_Normal", 1);
	List->SetFrameBufferTexture(Gbuffer, "GBUFFER_BaseSpec", 2);
	List->SetConstantBufferView(CBV, 0, "ConstData");
	GetShadowDataFromInput(2)->BindPointArray(List, "ShadowData");
	List->SetTexture(SceneRenderer::Get()->GetScene()->GetLightingData()->SkyBox, "SpecularBlurMap");
	SceneRenderer::Get()->BindLightsBuffer(List, "LightBuffer");
	List->SetBuffer(SceneRenderer::Get()->GetLightCullingEngine()->GetLightDataBuffer()->Get(List), "LightList");
	List->SetUAV(Target, "gOutput");
	List->SetConstantBufferView(SceneRenderer::Get()->GetVoxelScene()->TopLevelAcc->VoxelMapControlBuffer, 0, "VoxelDataBuffer");
	DynamicQualityEngine::Get()->BindRTBuffer(List, "RTBufferData");
	int index = List->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName("voxelTex");
	List->SetTexture2(SceneRenderer::Get()->GetVoxelScene()->TopLevelAcc->GetVoxelBuffer(), index, RHIViewDesc::DefaultSRV(DIMENSION_TEXTURE3D));
	List->DispatchSized(Target->GetWidth(), Target->GetHeight(), 1);
	List->UAVBarrier(Target);
}

void VoxelReflectionsNode::ExecuteVoxelConeTracing(FrameBuffer* Target, FrameBuffer* Gbuffer)
{
	DECALRE_SCOPEDGPUCOUNTER(List, "Voxel Cone Trace");
	DataSet.CameraPos = SceneRenderer::Get()->GetCurrentCamera()->GetPosition();
	DataSet.res = glm::ivec2(Target->GetWidth(), Target->GetHeight());
	DataSet.FrameCount = RHI::GetFrameCount();
	CBV->UpdateConstantBuffer(&DataSet);
	RHIPipeLineStateDesc PsoDesc = RHIPipeLineStateDesc::CreateDefault(ConeTraceShader);
	List->SetPipelineStateDesc(PsoDesc);
	List->SetFrameBufferTexture(Gbuffer, "GBUFFER_Pos", 0);
	List->SetFrameBufferTexture(Gbuffer, "GBUFFER_Normal", 1);
	List->SetFrameBufferTexture(Gbuffer, "GBUFFER_BaseSpec", 2);
	List->SetConstantBufferView(CBV, 0, "ConstData");
	List->SetUAV(Target, "gOutput");//
	List->SetConstantBufferView(SceneRenderer::Get()->GetVoxelScene()->TopLevelAcc->VoxelMapControlBuffer, 0, "VoxelDataBuffer");
	DynamicQualityEngine::Get()->BindRTBuffer(List, "RTBufferData");
	int index = List->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName("voxelTex");
	List->SetTexture2(SceneRenderer::Get()->GetVoxelScene()->TopLevelAcc->GetVoxelBuffer(), index, RHIViewDesc::DefaultSRV(DIMENSION_TEXTURE3D));
	index = List->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName("VoxelTransferMap");
	List->SetTexture2(SceneRenderer::Get()->GetVoxelScene()->TopLevelAcc->VoxelAlphaMap, index, RHIViewDesc::DefaultSRV(DIMENSION_TEXTURE3D));
	List->DispatchSized(Target->GetWidth(), Target->GetHeight(), 1);
	List->UAVBarrier(Target);
}

void VoxelReflectionsNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::UAV, StorageFormats::DefaultFormat, "OutputBuffer");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::GBufferData, "GBuffer");
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData, "Shadows");
	AddOutput(EStorageType::Framebuffer, StorageFormats::ScreenReflectionData, "Screen Data");
	LinkThough(0);
}

void VoxelReflectionsNode::OnSetupNode()
{
	Traceshader = new Shader_Pair(Context, { "Voxel\\VoxelReflections_CS" }, { EShaderType::SHADER_COMPUTE }, { ShaderProgramBase::Shader_Define("MAX_POINT_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, 1))) });
	ConeTraceShader = new Shader_Pair(Context, { "Voxel\\VoxelReflectionsTrace_CS" }, { EShaderType::SHADER_COMPUTE });
	List = RHI::CreateCommandList(ECommandListType::Compute, Context);
	CBV = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	CBV->CreateConstantBuffer(sizeof(ShaderData), 1);
}
