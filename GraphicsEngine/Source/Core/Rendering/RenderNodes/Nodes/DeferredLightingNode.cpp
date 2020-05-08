#include "DeferredLightingNode.h"
#include "Core/Assets/ShaderComplier.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/LightCulling/LightCullingEngine.h"
#include "Rendering/Core/ReflectionEnviroment.h"
#include "Rendering/Core/Screen.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/RenderNodes/StoreNodes/FrameBufferStorageNode.h"
#include "Rendering/RenderNodes/StoreNodes/ShadowAtlasStorageNode.h"
#include "Rendering/Shaders/Shader_Deferred.h"
#include "Rendering/Shaders/Shader_Pair.h"
#include "Rendering/Shaders/Shader_Skybox.h"
#include "RHI/RHITimeManager.h"
#include "../../Core/VRXEngine.h"
#include "../../RayTracing/Voxel/VoxelTracingEngine.h"
#include "../../Renderers/Terrain/TerrainRenderer.h"
#include "RHI/RHITexture.h"
#include "RHI/SFRController.h"

DeferredLightingNode::DeferredLightingNode()
{
	ViewMode = EViewMode::PerView;
	UseScreenSpaceReflection = true;
	OnNodeSettingChange();
}

DeferredLightingNode::~DeferredLightingNode()
{

}

void DeferredLightingNode::OnExecute()
{
	NodeLink* VRXImage = GetInputLinkByName("VRX Image");
	FrameBuffer* GBuffer = GetFrameBufferFromInput(NodeInputStruct.GBuffer);
	FrameBuffer* MainBuffer = GetFrameBufferFromInput(1);
	DeferredShader = ShaderComplier::GetShader<Shader_Deferred, int>(Context, MainBuffer->GetDescription().VarRateSettings.BufferMode);
	RHICommandList* List = GetListAndReset();
	if (VRXImage != nullptr && VRXImage->IsValid())
	{
		List->SetVRXShadingRateImage(StorageNode::NodeCast<FrameBufferStorageNode>(VRXImage->GetStoreTarget())->GetFramebuffer()->GetRenderTexture());
		//List->PrepareFramebufferForVRR(List->GetShadingRateImage(), MainBuffer);
	}
	List->StartTimer(EGPUTIMERS::DeferredLighting);

	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = DeferredShader;
	desc.RenderTargetDesc = MainBuffer->GetPiplineRenderDesc();
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRR())
	{
		desc.DepthStencilState.StencilEnable = false;
		desc.DepthStencilState.BackFace.StencilFunc = COMPARISON_FUNC_EQUAL;
		desc.DepthStencilState.FrontFace.StencilFunc = COMPARISON_FUNC_EQUAL;
	}
	List->SetPipelineStateDesc(desc);
	if (VRXImage != nullptr && VRXImage->IsValid())
	{
		RHITexture* Text = StorageNode::NodeCast<FrameBufferStorageNode>(VRXImage->GetStoreTarget())->GetFramebuffer()->GetRenderTexture();
		List->SetVRXShadingRateImage(Text);
		List->SetTexture2(Text, DeferredLightingShaderRSBinds::RateImage);
#if USEPS_VRR
		if (VRXEngine::Get()->TempTexture == nullptr)
		{
			RHITextureDesc2 Desc2 = MainBuffer->GetDescription().RenderTargets[0]->GetDescription();
			VRXEngine::Get()->TempTexture = RHI::GetRHIClass()->CreateTexture2();
			Desc2.AllowUnorderedAccess = true;
			VRXEngine::Get()->TempTexture->Create(Desc2);
		}
		VRXEngine::Get()->TempTexture->SetState(List, EResourceState::UAV);
		List->SetUAV(VRXEngine::Get()->TempTexture, DeferredLightingShaderRSBinds::TempVRRBuffer, RHIViewDesc::DefaultUAV());
#endif
	}
	RHIRenderPassDesc D = RHIRenderPassDesc(MainBuffer, RHI::GetRenderSettings()->GetVRXSettings().UseVRR() ? ERenderPassLoadOp::Load : ERenderPassLoadOp::Clear);
	List->BeginRenderPass(D);
	if (RHI::GetRenderSettings()->GetCurrnetSFRSettings().Enabled)
	{
		List->SetScissorRect(SFRController::GetScissor(List->GetDeviceIndex(), Screen::GetScaledRes()));
	}
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::PosTex, 0);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::NormalTex, 1);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::AlbedoTex, 2);
	if (UseScreenSpaceReflection && NodeInputStruct.SSRData->IsValid())
	{
		FrameBuffer* ScreenSpaceData = GetFrameBufferFromInput(NodeInputStruct.SSRData);
		List->SetFrameBufferTexture(ScreenSpaceData, DeferredLightingShaderRSBinds::ScreenSpecular);
	}
	if (NodeInputStruct.ShadowMask->IsValid())
	{
		List->SetFrameBufferTexture(GetFrameBufferFromInput(NodeInputStruct.ShadowMask) , DeferredLightingShaderRSBinds::PreSampleShadows);
	}
	SceneRenderer::Get()->GetLightCullingEngine()->BindLightBuffer(List, true);
	SceneRenderer::Get()->GetReflectionEnviroment()->BindStaticSceneEnivoment(List, true);
	//SceneRenderer::Get()->GetReflectionEnviroment()->BindDynamicReflections(List, true);
	SceneRenderer::Get()->BindLightsBufferB(List, DeferredLightingShaderRSBinds::LightDataCBV);
	SceneRenderer::Get()->BindMvBufferB(List, DeferredLightingShaderRSBinds::MVCBV, GetEye());
	if (NodeInputStruct.ShadowMaps->IsValid() && RHI::IsD3D12())
	{
		GetShadowDataFromInput(NodeInputStruct.ShadowMaps)->BindPointArray(List, 6);
	}
	SceneRenderer::DrawScreenQuad(List);
	List->EndRenderPass();


	Shader_Skybox* SkyboxShader = ShaderComplier::GetShader<Shader_Skybox>();
	SkyboxShader->Render(SceneRenderer::Get(), List, MainBuffer, GBuffer);
	List->EndTimer(EGPUTIMERS::DeferredLighting);
#if USEPS_VRR
	if (VRXImage != nullptr)
	{
		RHITexture* Text = StorageNode::NodeCast<FrameBufferStorageNode>(VRXImage->GetStoreTarget())->GetFramebuffer()->GetRenderTexture();
		VRXEngine::ResolveVRRFramebuffer_PS(List, MainBuffer, Text);
	}
#endif
	if (List->GetDeviceIndex() == 1)
	{
		List->GetDevice()->GetTimeManager()->EndTotalGPUTimer(List);
	}

	ExecuteList();
	GetInput(1)->GetStoreTarget()->DataFormat = StorageFormats::LitScene;

}

void DeferredLightingNode::OnNodeSettingChange()
{
	NodeInputStruct.GBuffer = AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::GBufferData, "GBuffer");
	NodeInputStruct.MainBuffer = AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::DefaultFormat, "Main buffer");
	NodeInputStruct.ShadowMaps = AddInput(EStorageType::ShadowData, StorageFormats::ShadowData, "Shadow Maps");
	if (UseScreenSpaceReflection)
	{
		NodeInputStruct.SSRData = AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::ScreenReflectionData, "SSR Data");
		NodeInputStruct.SSRData->SetOptional();
	}
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRX())
	{
		AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::ShadingImage, "VRX Image");
	}
	NodeInputStruct.ShadowMask = AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::PreSampleShadowData, "ShadowMask");

	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Lit scene");
	SetUseSeperateCommandList();
}
