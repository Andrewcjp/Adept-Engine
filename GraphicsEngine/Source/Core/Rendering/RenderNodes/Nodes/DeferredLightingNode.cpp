
#include "DeferredLightingNode.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNode.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../../Shaders/Shader_Deferred.h"
#include "../../Core/SceneRenderer.h"
#include "../../Core/LightCulling/LightCullingEngine.h"
#include "Core/Assets/Scene.h"
#include "Core/BaseWindow.h"
#include "../../Renderers/RenderEngine.h"
#include "../../Shaders/Generation/Shader_Convolution.h"
#include "../../Shaders/Generation/Shader_EnvMap.h"
#include "../../Shaders/Shader_Skybox.h"

DeferredLightingNode::DeferredLightingNode()
{
	ViewMode = EViewMode::PerView;
	OnNodeSettingChange();
}

DeferredLightingNode::~DeferredLightingNode()
{}

void DeferredLightingNode::OnSetupNode()
{
	List = RHI::CreateCommandList(ECommandListType::Graphics, Context);
	DeferredShader = new Shader_Deferred(Context);
}

void DeferredLightingNode::OnExecute()
{
	FrameBuffer* GBuffer = GetFrameBufferFromInput(0);
	FrameBuffer* MainBuffer = GetFrameBufferFromInput(1);
	Scene* MainScene = GetSceneDataFromInput(2);
	ensure(MainScene);
	
	List->ResetList();
	List->StartTimer(EGPUTIMERS::DeferredLighting);
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = DeferredShader;
	desc.FrameBufferTarget = MainBuffer;
	List->SetPipelineStateDesc(desc);

	RHIRenderPassDesc D = RHIRenderPassDesc(MainBuffer, ERenderPassLoadOp::Clear);
#if NOSHADOW
	D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
#endif
	List->BeginRenderPass(D);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::PosTex, 0);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::NormalTex, 1);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::AlbedoTex, 2);


	SceneRenderer::Get()->GetLightCullingEngine()->BindLightBuffer(List, true);

	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		/*if (RHI::GetRenderSettings()->RaytracingEnabled() && RHI::GetRenderSettings()->GetRTSettings().UseForReflections)
		{
			List->SetFrameBufferTexture(Object->RTBuffer, DeferredLightingShaderRSBinds::ScreenSpecular);
		}*/

		List->SetTexture(MainScene->GetLightingData()->SkyBox, DeferredLightingShaderRSBinds::SpecBlurMap);

		//List->SetFrameBufferTexture(SceneRender->probes[0]->CapturedTexture, DeferredLightingShaderRSBinds::SpecBlurMap);
	}
	//List->SetFrameBufferTexture(DDOs[List->GetDeviceIndex()].ConvShader->CubeBuffer, DeferredLightingShaderRSBinds::DiffuseIr);
	//List->SetFrameBufferTexture(DDOs[List->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, DeferredLightingShaderRSBinds::EnvBRDF);
#if 0
	CommandList->SetTexture(MainScene->GetLightingData()->SkyBox, DeferredLightingShaderRSBinds::DiffuseIr);
	CommandList->SetTexture(Defaults::GetDefaultTexture(), DeferredLightingShaderRSBinds::EnvBRDF);
#else
	if (BaseWindow::GetCurrentRenderer()->DDOs[List->GetDeviceIndex()].ConvShader != nullptr)
	{
		List->SetFrameBufferTexture(BaseWindow::GetCurrentRenderer()->DDOs[List->GetDeviceIndex()].ConvShader->CubeBuffer, DeferredLightingShaderRSBinds::DiffuseIr);
		List->SetFrameBufferTexture(BaseWindow::GetCurrentRenderer()->DDOs[List->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, DeferredLightingShaderRSBinds::EnvBRDF);
	}
#endif
	SceneRenderer::Get()->BindLightsBuffer(List, DeferredLightingShaderRSBinds::LightDataCBV);
	SceneRenderer::Get()->BindMvBuffer(List, DeferredLightingShaderRSBinds::MVCBV, 0);
	//SceneRender->BindMvBuffer(List, DeferredLightingShaderRSBinds::MVCBV, eyeindex);
#if !NOSHADOW
	//mShadowRenderer->BindShadowMapsToTextures(List);
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

#if !NOSHADOW
	//mShadowRenderer->Unbind(List);
#endif
	//	List->SetRenderTarget(nullptr);
	//if (List->GetDeviceIndex() == 0)
	//{
	//	DDOs[0].MainFrameBuffer->MakeReadyForCopy(List);
	//}
#if !NOSHADOW
	//RenderSkybox(List, output, GBuffer);
	Shader_Skybox* SkyboxShader = ShaderComplier::GetShader<Shader_Skybox>();
	SkyboxShader->Render(SceneRenderer::Get(), List, MainBuffer, GBuffer);
#endif
	/*GBuffer->MakeReadyForComputeUse(List, true);*/
	List->EndTimer(EGPUTIMERS::DeferredLighting);
	List->Execute();
	GetInput(1)->GetStoreTarget()->DataFormat = StorageFormats::LitScene;
	GetOutput(0)->SetStore(GetInput(1)->GetStoreTarget());
}

void DeferredLightingNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::GBufferData, "GBuffer");
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "Main buffer");
	AddInput(EStorageType::SceneData, StorageFormats::DefaultFormat, "Scene Data");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Lit scene");
}
