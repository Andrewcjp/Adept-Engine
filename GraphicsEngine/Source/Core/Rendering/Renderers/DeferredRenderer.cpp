#include "DeferredRenderer.h"
#include "Rendering/Core/Mesh.h"
#include "RHI/RHI.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Core/Assets/Scene.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "Editor/Editor_Camera.h"
#include "Rendering\Shaders\Shader_Skybox.h"
#include "Rendering/Shaders/Generation/Shader_EnvMap.h"
#include "Rendering/Shaders/Generation/Shader_Convolution.h"
#include "RHI/DeviceContext.h"
#include "Rendering/Core/SceneRenderer.h"

void DeferredRenderer::OnRender()
{
#if WITH_EDITOR
	if (EditorCam != nullptr && EditorCam->GetEnabled())
	{
		if (MainCamera != EditorCam->GetCamera())
		{
			MainCamera = EditorCam->GetCamera();
		}
	}
	else
	{
		if (MainCamera == EditorCam->GetCamera())
		{
			MainCamera = MainScene->GetCurrentRenderCamera();
		}
	}
#endif
	ShadowPass();
	GeometryPass();
	LightingPass();
	RenderSkybox();
	PostProcessPass();
}

void DeferredRenderer::RenderSkybox()
{
	//SkyBox->Render(SceneRender, FilterBuffer, GFrameBuffer);
}

void DeferredRenderer::PostInit()
{
	FilterBuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateColour(m_width, m_height));
	DeferredShader = ShaderComplier::GetShader<Shader_Deferred>();
	GFrameBuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateGBuffer(m_width, m_height));
	WriteList = RHI::CreateCommandList(ECommandListType::Graphics, RHI::GetDeviceContext(0));
	RHIPipeLineStateDesc desc;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = GFrameBuffer;
	WriteList->SetPipelineStateDesc(desc);
	LightingList = RHI::CreateCommandList(ECommandListType::Graphics, RHI::GetDeviceContext(0));
	desc = RHIPipeLineStateDesc();
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = DeferredShader;
	desc.FrameBufferTarget = FilterBuffer;
	LightingList->SetPipelineStateDesc(desc);
	/*SkyBox = ShaderComplier::GetShader<Shader_Skybox>();
	SkyBox->Init(FilterBuffer, GFrameBuffer);*/
}

void DeferredRenderer::GeometryPass()
{
	if (MainScene->StaticSceneNeedsUpdate)
	{
		SceneRender->UpdateLightBuffer(*MainScene->GetLights());
		PrepareData();
		SceneRender->UpdateCBV();
	}
	SceneRender->UpdateMV(MainCamera);
	WriteList->ResetList();
	WriteList->GetDevice()->GetTimeManager()->StartTotalGPUTimer(WriteList);
	WriteList->GetDevice()->GetTimeManager()->StartTimer(WriteList, EGPUTIMERS::DeferredWrite);
	WriteList->SetRenderTarget(GFrameBuffer);
	WriteList->ClearFrameBuffer(GFrameBuffer);
	SceneRender->RenderScene(WriteList, false, GFrameBuffer);
	WriteList->SetRenderTarget(nullptr);
	WriteList->GetDevice()->GetTimeManager()->EndTimer(WriteList, EGPUTIMERS::DeferredWrite);
	WriteList->Execute();
}

void DeferredRenderer::SSAOPass()
{
	//SSAOBuffer->BindBufferAsRenderTarget();
	//glClear(GL_COLOR_BUFFER_BIT);
	//->BindToTextureUnit();
	//SSAOShader->SetShaderActive();
	//SSAOShader->UpdateUniforms(nullptr, MainCamera, Lights);
	//SSAOShader->RenderPlane();
	//SSAOBuffer->UnBind();
}

void DeferredRenderer::LightingPass()
{
	LightingList->ResetList();
	WriteList->GetDevice()->GetTimeManager()->StartTimer(LightingList, EGPUTIMERS::DeferredLighting);

	LightingList->SetRenderTarget(FilterBuffer);
	LightingList->ClearFrameBuffer(FilterBuffer);
	LightingList->SetFrameBufferTexture(GFrameBuffer, DeferredLightingShaderRSBinds::PosTex, 0);
	LightingList->SetFrameBufferTexture(GFrameBuffer, DeferredLightingShaderRSBinds::NormalTex, 1);
	LightingList->SetFrameBufferTexture(GFrameBuffer, DeferredLightingShaderRSBinds::AlbedoTex, 2);
	LightingList->SetFrameBufferTexture(DDOs[0].ConvShader->CubeBuffer, DeferredLightingShaderRSBinds::DiffuseIr);
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		LightingList->SetTexture(MainScene->GetLightingData()->SkyBox, DeferredLightingShaderRSBinds::SpecBlurMap);
	}
	LightingList->SetFrameBufferTexture(DDOs[0].EnvMap->EnvBRDFBuffer, DeferredLightingShaderRSBinds::EnvBRDF);

	SceneRender->BindLightsBuffer(LightingList, DeferredLightingShaderRSBinds::LightDataCBV);
	SceneRender->BindMvBuffer(LightingList, DeferredLightingShaderRSBinds::MVCBV);

	mShadowRenderer->BindShadowMapsToTextures(LightingList);

	DeferredShader->RenderScreenQuad(LightingList);
	WriteList->GetDevice()->GetTimeManager()->EndTimer(LightingList, EGPUTIMERS::DeferredLighting);
	mShadowRenderer->Unbind(LightingList);
	LightingList->Execute();
}

void DeferredRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	FilterBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	GFrameBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	RenderEngine::Resize(width, height);
	if (MainCamera != nullptr)
	{
		MainCamera->UpdateProjection((float)width / (float)height);
	}
}

DeferredRenderer::~DeferredRenderer()
{}

void DeferredRenderer::DestoryRenderWindow()
{
	EnqueueSafeRHIRelease(GFrameBuffer);
	EnqueueSafeRHIRelease(WriteList);
	EnqueueSafeRHIRelease(LightingList);
	EnqueueSafeRHIRelease(OutputBuffer);
}

void DeferredRenderer::FinaliseRender()
{}

void DeferredRenderer::OnStaticUpdate()
{}
