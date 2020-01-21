#include "VoxelReflectionsNode.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_Pair.h"
#include "../../Core/FrameBuffer.h"
#include "../../RayTracing/VoxelTracingEngine.h"
#include "../../Core/SceneRenderer.h"
#include "../../Core/Screen.h"
#include "../../Core/Camera.h"
#include "RHI/RHITimeManager.h"
#include "../../Core/LightCulling/LightCullingEngine.h"
#include "RHI/RHICommandList.h"
#include "Core/Assets/Scene.h"
#include "../../Core/DynamicQualityEngine.h"
#include "../StoreNodes/ShadowAtlasStorageNode.h"

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
		GetShadowDataFromInput(2)->BindPointArray(List,"ShadowData");
		List->SetTexture(SceneRenderer::Get()->GetScene()->GetLightingData()->SkyBox, "SpecularBlurMap");
		SceneRenderer::Get()->BindLightsBuffer(List, "LightBuffer");
		List->SetBuffer(SceneRenderer::Get()->GetLightCullingEngine()->GetLightDataBuffer(), "LightList");
		List->SetUAV(Target, "gOutput");
		DynamicQualityEngine::Get()->BindRTBuffer(List, "RTBufferData");
		int index = List->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName("voxelTex");
		List->SetTexture2(VoxelTracingEngine::Get()->VoxelMap, index, RHIViewDesc::DefaultSRV(DIMENSION_TEXTURE3D));
		List->DispatchSized(Target->GetWidth(), Target->GetHeight(), 1);
		List->UAVBarrier(Target);
	}
	SetEndStates(List);
	List->Execute();
	FLAT_COMPUTE_END(List->GetDevice());
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
	List = RHI::CreateCommandList(ECommandListType::Compute, Context);
	CBV = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	CBV->CreateConstantBuffer(sizeof(ShaderData), 1);
}
