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
	FrameBuffer* GBuffer = GetFrameBufferFromInput(0);
	FrameBuffer* MainBuffer = GetFrameBufferFromInput(1);
	Scene* MainScene = GetSceneDataFromInput(2);
	ensure(MainScene);
	DeferredShader = ShaderComplier::GetShader<Shader_Deferred, int>(Context, MainBuffer->GetDescription().VarRateSettings.BufferMode);
	List->ResetList();
	List->StartTimer(EGPUTIMERS::DeferredLighting);
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = DeferredShader;
	desc.RenderTargetDesc = MainBuffer->GetPiplineRenderDesc();
	List->SetPipelineStateDesc(desc);

	RHIRenderPassDesc D = RHIRenderPassDesc(MainBuffer, ERenderPassLoadOp::Clear);
#if NOSHADOW
	D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
#endif
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
	SceneRenderer::Get()->BindLightsBuffer(List, DeferredLightingShaderRSBinds::LightDataCBV);
	SceneRenderer::Get()->BindMvBuffer(List, DeferredLightingShaderRSBinds::MVCBV, GetEye());

	if (GetInput(3)->IsValid() && RHI::IsD3D12())
	{
		GetShadowDataFromInput(3)->BindPointArray(List, 6);
	}
#if TEST_VRR
	List->SetVRRShadingRate(2);
#endif
	DeferredShader->RenderScreenQuad(List);

	//transparent pass
	//if (RHI::GetRenderSettings()->GetSettingsForRender().EnableTransparency)
	//{
	//	GBuffer->BindDepthWithColourPassthrough(List, output);
	//	SceneRender->SetupBindsForForwardPass(List, eyeindex);
	//	SceneRender->MeshController->RenderPass(ERenderPass::TransparentPass, List); 
	//}
	List->EndRenderPass();

#if !TEST_VRR
	Shader_Skybox* SkyboxShader = ShaderComplier::GetShader<Shader_Skybox>();
	SkyboxShader->Render(SceneRenderer::Get(), List, MainBuffer, GBuffer);
#endif
	List->EndTimer(EGPUTIMERS::DeferredLighting);
	SetEndStates(List);
	//	GBuffer->SetResourceState(List, EResourceState::ComputeUse);
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

}
