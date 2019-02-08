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
	if (RHI::GetMGPUMode()->MainPassSFR)
	{
		RenderOnDevice(RHI::GetDeviceContext(1));
	}
	RenderOnDevice(RHI::GetDeviceContext(0));
	if (DevicesInUse > 1)
	{
		DDDOs[1].OutputBuffer->ResolveSFR(FilterBuffer);
	}
	PostProcessPass();
}

void DeferredRenderer::RenderOnDevice(DeviceContext* con)
{
	DeferredDeviceObjects* d = &DDDOs[con->GetDeviceIndex()];
	GeometryPass(d->WriteList);
	LightingPass(d->LightingList);
	RenderSkybox(con);
}

void DeferredRenderer::RenderSkybox(DeviceContext* con)
{
	DDOs[con->GetDeviceIndex()].SkyboxShader->Render(SceneRender, DDDOs[con->GetDeviceIndex()].OutputBuffer, DDDOs[con->GetDeviceIndex()].GFrameBuffer);
}

void DeferredRenderer::PostInit()
{
	SetUpOnDevice(RHI::GetDeviceContext(0));
	if (RHI::GetMGPUMode()->MainPassSFR)
	{
		SetUpOnDevice(RHI::GetDeviceContext(1));
	}
}

void DeferredRenderer::SetUpOnDevice(DeviceContext* con)
{
	DeferredDeviceObjects* DDO = &DDDOs[con->GetDeviceIndex()];
	RHIFrameBufferDesc FBDesc = RHIFrameBufferDesc::CreateColour(m_width, m_height);
	FBDesc.IncludedInSFR = true;
	if (con->GetDeviceIndex() > 0)
	{
		FBDesc.IsShared = true;
		FBDesc.DeviceToCopyTo = RHI::GetDeviceContext(0);
	}
	DDO->OutputBuffer = RHI::CreateFrameBuffer(con, FBDesc);
	if (con->GetDeviceIndex() == 0)
	{
		FilterBuffer = DDO->OutputBuffer;
	}
	DDO->DeferredShader = new Shader_Deferred(con);
	FBDesc = RHIFrameBufferDesc::CreateGBuffer(m_width, m_height);
	FBDesc.IncludedInSFR = true;
	DDO->GFrameBuffer = RHI::CreateFrameBuffer(con, FBDesc);
	DDO->WriteList = RHI::CreateCommandList(ECommandListType::Graphics, con);
	RHIPipeLineStateDesc desc;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = DDO->GFrameBuffer;
	DDO->WriteList->SetPipelineStateDesc(desc);
	DDO->LightingList = RHI::CreateCommandList(ECommandListType::Graphics, con);
	desc = RHIPipeLineStateDesc();
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = DDO->DeferredShader;
	desc.FrameBufferTarget = DDO->OutputBuffer;
	DDO->LightingList->SetPipelineStateDesc(desc);
	DDOs[con->GetDeviceIndex()].SkyboxShader = new Shader_Skybox(con);// ShaderComplier::GetShader<Shader_Skybox>();
	DDOs[con->GetDeviceIndex()].SkyboxShader->Init(DDO->OutputBuffer, DDO->GFrameBuffer);

}

void DeferredRenderer::GeometryPass(RHICommandList* List)
{
	if (MainScene->StaticSceneNeedsUpdate)
	{
		SceneRender->UpdateLightBuffer(*MainScene->GetLights());
		PrepareData();
		SceneRender->UpdateCBV();
	}
	SceneRender->UpdateMV(MainCamera);
	List->ResetList();
	List->GetDevice()->GetTimeManager()->StartTotalGPUTimer(List);
	List->GetDevice()->GetTimeManager()->StartTimer(List, EGPUTIMERS::DeferredWrite);
	List->SetRenderTarget(DDDOs[List->GetDeviceIndex()].GFrameBuffer);
	List->ClearFrameBuffer(DDDOs[List->GetDeviceIndex()].GFrameBuffer);
	SceneRender->RenderScene(List, false, DDDOs[List->GetDeviceIndex()].GFrameBuffer);
	List->SetRenderTarget(nullptr);
	List->GetDevice()->GetTimeManager()->EndTimer(List, EGPUTIMERS::DeferredWrite);
	List->Execute();
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

void DeferredRenderer::LightingPass(RHICommandList* List)
{
	List->ResetList();
	List->GetDevice()->GetTimeManager()->StartTimer(List, EGPUTIMERS::DeferredLighting);

	List->SetRenderTarget(DDDOs[List->GetDeviceIndex()].OutputBuffer);
	List->ClearFrameBuffer(DDDOs[List->GetDeviceIndex()].OutputBuffer);
	List->SetFrameBufferTexture(DDDOs[List->GetDeviceIndex()].GFrameBuffer, DeferredLightingShaderRSBinds::PosTex, 0);
	List->SetFrameBufferTexture(DDDOs[List->GetDeviceIndex()].GFrameBuffer, DeferredLightingShaderRSBinds::NormalTex, 1);
	List->SetFrameBufferTexture(DDDOs[List->GetDeviceIndex()].GFrameBuffer, DeferredLightingShaderRSBinds::AlbedoTex, 2);
	List->SetFrameBufferTexture(DDOs[List->GetDeviceIndex()].ConvShader->CubeBuffer, DeferredLightingShaderRSBinds::DiffuseIr);
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		List->SetTexture(MainScene->GetLightingData()->SkyBox, DeferredLightingShaderRSBinds::SpecBlurMap);
	}
	List->SetFrameBufferTexture(DDOs[List->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, DeferredLightingShaderRSBinds::EnvBRDF);

	SceneRender->BindLightsBuffer(List, DeferredLightingShaderRSBinds::LightDataCBV);
	SceneRender->BindMvBuffer(List, DeferredLightingShaderRSBinds::MVCBV);

	mShadowRenderer->BindShadowMapsToTextures(List);

	DDDOs[List->GetDeviceIndex()].DeferredShader->RenderScreenQuad(List);
	List->GetDevice()->GetTimeManager()->EndTimer(List, EGPUTIMERS::DeferredLighting);
	mShadowRenderer->Unbind(List);
	List->SetRenderTarget(nullptr);
	if (List->GetDeviceIndex() == 0)
	{
		FilterBuffer->MakeReadyForCopy(List);
	}
	List->Execute();
}

void DeferredRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	FilterBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DDDOs[i].GFrameBuffer)
		{
			DDDOs[i].GFrameBuffer->Resize(GetScaledWidth(), GetScaledHeight());
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
	//EnqueueSafeRHIRelease(GFrameBuffer);
	//EnqueueSafeRHIRelease(WriteList);
	//EnqueueSafeRHIRelease(LightingList);
//	EnqueueSafeRHIRelease(OutputBuffer);
}

void DeferredRenderer::FinaliseRender()
{}

void DeferredRenderer::OnStaticUpdate()
{}
