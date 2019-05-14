#include "DeferredRenderer.h"
#include "Core/Assets/Scene.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Editor/Editor_Camera.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Shaders/Generation/Shader_EnvMap.h"
#include "Rendering/Shaders/Shader_Skybox.h"
#include "RHI/DeviceContext.h"
#include "../Shaders/PostProcess/Shader_DebugOutput.h"
#include "../Core/ParticleSystemManager.h"
#include "../VR/HMD.h"
#include "RenderEngine.h"
#include "../Core/RelfectionProbe.h"

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
	CubeMapPass();
	if (RHI::GetMGPUSettings()->MainPassSFR)
	{
		RenderOnDevice(RHI::GetDeviceContext(1));
	}
	RenderOnDevice(RHI::GetDeviceContext(0));
	ParticleSystemManager::Get()->Render(DDOs[0].MainFrameBuffer, DDOs[0].Gbuffer);
	if (DevicesInUse > 1)
	{
		DDOs[1].MainFrameBuffer->ResolveSFR(DDOs[0].MainFrameBuffer);
	}
	//PostProcessPass();
	PresentToScreen();
}

void DeferredRenderer::RenderOnDevice(DeviceContext* con)
{
	DeviceDependentObjects* d = &DDOs[con->GetDeviceIndex()];
	d->GbufferWriteList->ResetList();
	UpdateMVForMainPass();
	GeometryPass(d->GbufferWriteList, d->Gbuffer);
	if (RHI::IsRenderingVR())
	{
		GeometryPass(d->GbufferWriteList, d->RightEyeGBuffer,EEye::Right);
	}
	d->GbufferWriteList->Execute();
#if ENABLE_RENDERER_DEBUGGING
	if (RHI::GetRenderSettings()->GetDebugRenderMode() == ERenderDebugOutput::Off)
	{
#endif
		d->MainCommandList->ResetList();
		LightingPass(d->MainCommandList, d->Gbuffer, d->MainFrameBuffer);
		if (RHI::IsRenderingVR())
		{
			LightingPass(d->MainCommandList, d->RightEyeGBuffer, d->RightEyeFramebuffer);
		}
		d->MainCommandList->Execute();
#if ENABLE_RENDERER_DEBUGGING
	}
	else
	{
		DebugPass();
	}
#endif


}

void DeferredRenderer::RenderSkybox(RHICommandList* list, FrameBuffer* Output, FrameBuffer* DepthSource)
{
	DDOs[list->GetDeviceIndex()].SkyboxShader->Render(SceneRender, list, Output, DepthSource);
}

void DeferredRenderer::PostInit()
{
	SetUpOnDevice(RHI::GetDeviceContext(0));
	if (RHI::GetMGPUSettings()->MainPassSFR)
	{
		SetUpOnDevice(RHI::GetDeviceContext(1));
	}
}

void DeferredRenderer::SetUpOnDevice(DeviceContext* con)
{
	DeviceDependentObjects* DDO = &DDOs[con->GetDeviceIndex()];
	RHIFrameBufferDesc FBDesc = RHIFrameBufferDesc::CreateColour(m_width, m_height);
	FBDesc.IncludedInSFR = true;
	if (con->GetDeviceIndex() > 0)
	{
		FBDesc.IsShared = true;
		FBDesc.DeviceToCopyTo = RHI::GetDeviceContext(0);
	}
	DDO->MainFrameBuffer = RHI::CreateFrameBuffer(con, FBDesc);
	if (RHI::SupportVR())
	{
		DDO->RightEyeFramebuffer = RHI::CreateFrameBuffer(con, FBDesc);
	}
	if (con->GetDeviceIndex() == 0)
	{
		DDOs[0].MainFrameBuffer = DDO->MainFrameBuffer;
	}
	DDO->DeferredShader = new Shader_Deferred(con);
	FBDesc = RHIFrameBufferDesc::CreateGBuffer(m_width, m_height);
	FBDesc.IncludedInSFR = true;
	DDO->Gbuffer = RHI::CreateFrameBuffer(con, FBDesc);
	if (RHI::SupportVR())
	{
		DDO->RightEyeGBuffer = RHI::CreateFrameBuffer(con, FBDesc);
	}
	DDO->GbufferWriteList = RHI::CreateCommandList(ECommandListType::Graphics, con);

	DDO->MainCommandList = RHI::CreateCommandList(ECommandListType::Graphics, con);

	DDOs[con->GetDeviceIndex()].SkyboxShader = new Shader_Skybox(con);// ShaderComplier::GetShader<Shader_Skybox>();
	DDOs[con->GetDeviceIndex()].SkyboxShader->Init(DDO->MainFrameBuffer, DDO->Gbuffer);

	if (con->GetDeviceIndex() == 0)
	{
		DebugList = RHI::CreateCommandList();
		RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
		desc.InitOLD(false, false, false);
		desc.ShaderInUse = ShaderComplier::GetShader<Shader_DebugOutput>();
		desc.FrameBufferTarget = DDO->MainFrameBuffer;
		DebugList->SetPipelineStateDesc(desc);
	}
}

void DeferredRenderer::GeometryPass(RHICommandList* List, FrameBuffer* gbuffer, int eyeindex)
{
	if (MainScene->StaticSceneNeedsUpdate)
	{
		SceneRender->UpdateLightBuffer(*MainScene->GetLights());
		PrepareData();
	}


	RHIPipeLineStateDesc desc;
	desc.DepthStencilState.DepthWrite = true;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = gbuffer;
	List->SetPipelineStateDesc(desc);
	List->GetDevice()->GetTimeManager()->StartTotalGPUTimer(List);
	List->GetDevice()->GetTimeManager()->StartTimer(List, EGPUTIMERS::DeferredWrite);
	List->SetRenderTarget(gbuffer);
	List->ClearFrameBuffer(gbuffer);
	SceneRender->RenderScene(List, false, gbuffer, false, eyeindex);
	List->SetRenderTarget(nullptr);
	List->GetDevice()->GetTimeManager()->EndTimer(List, EGPUTIMERS::DeferredWrite);

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
#if ENABLE_RENDERER_DEBUGGING
void DeferredRenderer::DebugPass()
{
	DebugList->ResetList();
	DebugList->SetRenderTarget(DDOs[DebugList->GetDeviceIndex()].MainFrameBuffer);
	DebugList->ClearFrameBuffer(DDOs[DebugList->GetDeviceIndex()].MainFrameBuffer);
	int currentDebugType = RHI::GetRenderSettings()->GetDebugRenderMode();
	DebugList->SetFrameBufferTexture(DDOs[DebugList->GetDeviceIndex()].Gbuffer, 0, currentDebugType - 1);
	DDOs[DebugList->GetDeviceIndex()].DeferredShader->RenderScreenQuad(DebugList);
	DebugList->Execute();
}
#endif
void DeferredRenderer::LightingPass(RHICommandList* List, FrameBuffer* GBuffer, FrameBuffer* output)
{

	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = DDOs[List->GetDeviceIndex()].DeferredShader;
	desc.FrameBufferTarget = DDOs[List->GetDeviceIndex()].MainFrameBuffer;
	List->SetPipelineStateDesc(desc);
	List->GetDevice()->GetTimeManager()->StartTimer(List, EGPUTIMERS::DeferredLighting);
	List->SetRenderTarget(output);
	List->ClearFrameBuffer(output);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::PosTex, 0);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::NormalTex, 1);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::AlbedoTex, 2);
	List->SetFrameBufferTexture(DDOs[List->GetDeviceIndex()].ConvShader->CubeBuffer, DeferredLightingShaderRSBinds::DiffuseIr);
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		List->SetTexture(MainScene->GetLightingData()->SkyBox, DeferredLightingShaderRSBinds::SpecBlurMap);
		//List->SetFrameBufferTexture(SceneRender->probes[0]->CapturedTexture, DeferredLightingShaderRSBinds::SpecBlurMap);
	}
	List->SetFrameBufferTexture(DDOs[List->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, DeferredLightingShaderRSBinds::EnvBRDF);

	SceneRender->BindLightsBuffer(List, DeferredLightingShaderRSBinds::LightDataCBV);
	SceneRender->BindMvBuffer(List, DeferredLightingShaderRSBinds::MVCBV);

	mShadowRenderer->BindShadowMapsToTextures(List);

	DDOs[List->GetDeviceIndex()].DeferredShader->RenderScreenQuad(List);
	List->GetDevice()->GetTimeManager()->EndTimer(List, EGPUTIMERS::DeferredLighting);
	mShadowRenderer->Unbind(List);
	List->SetRenderTarget(nullptr);
	if (List->GetDeviceIndex() == 0)
	{
		DDOs[0].MainFrameBuffer->MakeReadyForCopy(List);
	}
	RenderSkybox(List, output, GBuffer);
}

void DeferredRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	DDOs[0].MainFrameBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DDOs[i].Gbuffer)
		{
			DDOs[i].Gbuffer->Resize(GetScaledWidth(), GetScaledHeight());
			DDOs[i].MainFrameBuffer->Resize(GetScaledWidth(), GetScaledHeight());
		}
	}

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
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		DDOs[i].Release();
	}
}

void DeferredRenderer::FinaliseRender()
{}

void DeferredRenderer::OnStaticUpdate()
{}
