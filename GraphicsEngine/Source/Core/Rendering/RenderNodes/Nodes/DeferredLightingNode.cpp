#include "DeferredLightingNode.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/LightCulling/LightCullingEngine.h"
#include "Rendering/Core/ReflectionEnviroment.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/RenderNodes/StoreNodes/ShadowAtlasStorageNode.h"
#include "Rendering/Shaders/Shader_Deferred.h"
#include "Rendering/Shaders/Shader_Skybox.h"
#include "Core/Assets/ShaderComplier.h"
#include "../StoreNodes/FrameBufferStorageNode.h"
#include "../../Shaders/Shader_Pair.h"
#include "../../Core/Screen.h"

DeferredLightingNode::DeferredLightingNode()
{
	ViewMode = EViewMode::PerView;
	OnNodeSettingChange();
}

DeferredLightingNode::~DeferredLightingNode()
{
	SafeRHIRelease(List);
}

void DeferredLightingNode::OnSetupNode()
{
	List = RHI::CreateCommandList(ECommandListType::Graphics, Context);
	//DeferredShader = new Shader_Deferred(Context);
}

void DeferredLightingNode::OnExecute()
{
	NodeLink* VRXImage = GetInputLinkByName("VRX Image");
	FrameBuffer* GBuffer = GetFrameBufferFromInput(0);
	FrameBuffer* MainBuffer = GetFrameBufferFromInput(1);
	Scene* MainScene = GetSceneDataFromInput(2);
	ensure(MainScene);
	DeferredShader = ShaderComplier::GetShader<Shader_Deferred, int>(Context, MainBuffer->GetDescription().VarRateSettings.BufferMode);
	List->ResetList();
	List->StartTimer(EGPUTIMERS::DeferredLighting);
	if (RHI::GetRenderSettings()->GetVRXSettings().EnableVRR)
	{
		StorageNode::NodeCast<FrameBufferStorageNode>(VRXImage->GetStoreTarget())->GetFramebuffer()->SetResourceState(List, EResourceState::PixelShader);
		//write the depth stencil
		RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
		desc.InitOLD(false, false, false);
		Shader_Pair* Pair = new Shader_Pair(Context, { "Deferred_LightingPass_vs" ,"VRX/VRRWriteStencil" }, { EShaderType::SHADER_VERTEX,EShaderType::SHADER_FRAGMENT },
			{ ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(Context->GetCaps().VRSTileSize)) });
		desc.ShaderInUse = Pair;
		desc.RenderTargetDesc = MainBuffer->GetPiplineRenderDesc();
		desc.DepthStencilState.StencilEnable = true;
		desc.DepthStencilState.BackFace.StencilFunc = COMPARISON_FUNC_EQUAL;
		desc.DepthStencilState.FrontFace.StencilFunc = COMPARISON_FUNC_EQUAL;
		//looks like discard acts as a stencil pass
		desc.DepthStencilState.FrontFace.StencilPassOp = STENCIL_OP_INCR;
		desc.DepthStencilState.BackFace.StencilPassOp = STENCIL_OP_INCR;
		List->SetPipelineStateDesc(desc);
		List->SetTexture2(StorageNode::NodeCast<FrameBufferStorageNode>(VRXImage->GetStoreTarget())->GetFramebuffer()->GetRenderTexture(), "RateImage");
		glm::ivec2 Resoloution = Screen::GetScaledRes();
		List->SetRootConstant("ResData", 2, &Resoloution);
		RHIRenderPassDesc D = RHIRenderPassDesc(MainBuffer, ERenderPassLoadOp::Clear);
		List->BeginRenderPass(D);
		SceneRenderer::DrawScreenQuad(List);
		List->EndRenderPass();
		StorageNode::NodeCast<FrameBufferStorageNode>(VRXImage->GetStoreTarget())->GetFramebuffer()->SetResourceState(List, EResourceState::Non_PixelShader);
	}


	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = DeferredShader;
	desc.RenderTargetDesc = MainBuffer->GetPiplineRenderDesc();
	if (RHI::GetRenderSettings()->GetVRXSettings().EnableVRR)
	{
		desc.DepthStencilState.StencilEnable = true;
		desc.DepthStencilState.BackFace.StencilFunc = COMPARISON_FUNC_EQUAL;
		desc.DepthStencilState.FrontFace.StencilFunc = COMPARISON_FUNC_EQUAL;
	}
	List->SetPipelineStateDesc(desc);

	RHIRenderPassDesc D = RHIRenderPassDesc(MainBuffer, RHI::GetRenderSettings()->GetVRXSettings().EnableVRR ? ERenderPassLoadOp::Load : ERenderPassLoadOp::Clear);
	List->BeginRenderPass(D);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::PosTex, 0);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::NormalTex, 1);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::AlbedoTex, 2);
	if (UseScreenSpaceReflection)
	{
		FrameBuffer* ScreenSpaceData = GetFrameBufferFromInput(4);
		ScreenSpaceData->SetResourceState(List, EResourceState::PixelShader);
		List->SetFrameBufferTexture(ScreenSpaceData, DeferredLightingShaderRSBinds::ScreenSpecular);
	}

	SceneRenderer::Get()->GetLightCullingEngine()->BindLightBuffer(List, true);
	SceneRenderer::Get()->GetReflectionEnviroment()->BindStaticSceneEnivoment(List, true);
	//SceneRenderer::Get()->GetReflectionEnviroment()->BindDynamicReflections(List, true);
	SceneRenderer::Get()->BindLightsBufferB(List, DeferredLightingShaderRSBinds::LightDataCBV);
	SceneRenderer::Get()->BindMvBufferB(List, DeferredLightingShaderRSBinds::MVCBV, GetEye());

	if (GetInput(3)->IsValid() && RHI::IsD3D12())
	{
		GetShadowDataFromInput(3)->BindPointArray(List, 6);
	}

	if (VRXImage != nullptr && VRXImage->IsValid())
	{
		//List->SetVRXShadingRateImage(StorageNode::NodeCast<FrameBufferStorageNode>(VRXImage->GetStoreTarget())->GetFramebuffer()->GetRenderTexture());
	}
	SceneRenderer::DrawScreenQuad(List);
	List->EndRenderPass();
#if !TEST_VRR
	Shader_Skybox* SkyboxShader = ShaderComplier::GetShader<Shader_Skybox>();
	SkyboxShader->Render(SceneRenderer::Get(), List, MainBuffer, GBuffer);
#endif
	List->EndTimer(EGPUTIMERS::DeferredLighting);
	SetEndStates(List);
	if (VRXImage != nullptr && VRXImage->IsValid())
	{
		//StorageNode::NodeCast<FrameBufferStorageNode>(VRXImage->GetStoreTarget())->GetFramebuffer()->SetResourceState(List, EResourceState::UAV);
	}
	List->Execute();
	GetInput(1)->GetStoreTarget()->DataFormat = StorageFormats::LitScene;
	GetOutput(0)->SetStore(GetInput(1)->GetStoreTarget());
}

void DeferredLightingNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::ComputeUse, StorageFormats::GBufferData, "GBuffer");
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "Main buffer");
	AddInput(EStorageType::SceneData, StorageFormats::DefaultFormat, "Scene Data");
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData, "Shadow Maps");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Lit scene");
	if (UseScreenSpaceReflection)
	{
		AddInput(EStorageType::Framebuffer, StorageFormats::ScreenReflectionData, "SSR Data");
	}
	if (RHI::GetRenderSettings()->GetVRXSettings().EnableVRR)
	{
		AddInput(EStorageType::Framebuffer, StorageFormats::ShadingImage, "VRX Image");
	}
	AddInput(EStorageType::Framebuffer, StorageFormats::PreSampleShadowData, "ShadowMask");
}
