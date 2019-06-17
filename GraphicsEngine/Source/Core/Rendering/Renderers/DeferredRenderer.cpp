#include "DeferredRenderer.h"
#include "Core/Assets/Scene.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Editor/Editor_Camera.h"
#include "RenderEngine.h"
#include "Rendering/Core/Defaults.h"
#include "Rendering/Core/ParticleSystemManager.h"
#include "Rendering/Core/RelfectionProbe.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/RayTracing/RayTracingEngine.h"
#include "Rendering/Shaders/Generation/Shader_EnvMap.h"
#include "Rendering/Shaders/PostProcess/Shader_DebugOutput.h"
#include "Rendering/Shaders/Shader_Skybox.h"
#include "Rendering/VR/HMD.h"
#include "Rendering/VR/VRCamera.h"
#include "RHI/DeviceContext.h"

void DeferredRenderer::OnRender()
{
	PrepareData();
	SceneRender->UpdateLightBuffer(MainScene->GetLights());
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
	RunLightCulling();
	ShadowPass();
	CubeMapPass();
	UpdateMVForMainPass();
	if (RHI::GetMGPUSettings()->MainPassSFR)
	{
		RenderOnDevice(RHI::GetDeviceContext(1), EEye::Left);
	}
	RenderOnDevice(RHI::GetDeviceContext(0), EEye::Left);
	if (RHI::IsRenderingVR())
	{
		RenderOnDevice(RHI::GetDeviceContext(0), EEye::Right);
	}

	ParticleSystemManager::Get()->Render(&DDOs[0], DDOs[0].Gbuffer);
	if (DevicesInUse > 1)
	{
		DDOs[1].MainFrameBuffer->ResolveSFR(DDOs[0].MainFrameBuffer);
	}
	PostProcessPass();
	RenderDebugPass();
	PresentToScreen();
}

void DeferredRenderer::RenderOnDevice(DeviceContext* con, EEye::Type Eye)
{
	DeviceDependentObjects* d = &DDOs[con->GetDeviceIndex()];
	RHICommandList* WriteList = d->GbufferWriteList[Eye];
	RHICommandList* MList = d->MainCommandList[Eye];
	WriteList->ResetList();
	WriteList->StartTimer(EGPUTIMERS::DeferredWrite);
	GeometryPass(WriteList, d->GetGBuffer(Eye), Eye);
	WriteList->EndTimer(EGPUTIMERS::DeferredWrite);
	WriteList->Execute();
	RunReflections(d);
#if ENABLE_RENDERER_DEBUGGING
	if (RHI::GetRenderSettings()->GetDebugRenderMode() == ERenderDebugOutput::Off)
	{
#endif
		MList->ResetList();
		MList->StartTimer(EGPUTIMERS::DeferredLighting);
		LightingPass(MList, d->GetGBuffer(Eye), d->GetMain(Eye), Eye);
		MList->EndTimer(EGPUTIMERS::DeferredLighting);
		MList->Execute();
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
	const float ratio = 1.0f;// 0.3;
	RHIFrameBufferDesc FBDesc = RHIFrameBufferDesc::CreateColour(GetScaledWidth()*ratio, GetScaledHeight()*ratio);
	FBDesc.IncludedInSFR = true;
	FBDesc.AllowUnordedAccess = true;
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
	FBDesc = RHIFrameBufferDesc::CreateGBuffer(GetScaledWidth(), GetScaledHeight());
	FBDesc.clearcolour = glm::vec4(0, 0, 0, 0);
	FBDesc.IncludedInSFR = true;
	DDO->Gbuffer = RHI::CreateFrameBuffer(con, FBDesc);
	DDO->Gbuffer->SetDebugName("GBuffer");
	if (RHI::SupportVR())
	{
		DDO->RightEyeGBuffer = RHI::CreateFrameBuffer(con, FBDesc);
	}
	DDO->GbufferWriteList[0] = RHI::CreateCommandList(ECommandListType::Graphics, con);
	DDO->MainCommandList[0] = RHI::CreateCommandList(ECommandListType::Graphics, con);
	DDO->GbufferWriteList[1] = RHI::CreateCommandList(ECommandListType::Graphics, con);
	DDO->MainCommandList[1] = RHI::CreateCommandList(ECommandListType::Graphics, con);

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
	if (RHI::GetRenderSettings()->RaytracingEnabled())
	{
		const float Scale = RHI::GetRenderSettings()->GetRTSettings().ReflectionBufferScale;
		FBDesc = RHIFrameBufferDesc::CreateColour(GetScaledWidth()*Scale, GetScaledHeight()*Scale);
		FBDesc.AllowUnordedAccess = true;
		FBDesc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		DDO->RTBuffer = RHI::CreateFrameBuffer(con, FBDesc);
	}
}

void DeferredRenderer::GeometryPass(RHICommandList* List, FrameBuffer* gbuffer, int eyeindex)
{
	RHIPipeLineStateDesc desc;
	desc.DepthStencilState.DepthWrite = true;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = gbuffer;
	List->SetPipelineStateDesc(desc);
	List->GetDevice()->GetTimeManager()->StartTotalGPUTimer(List);

	//List->SetRenderTarget(gbuffer);
	//List->ClearFrameBuffer(gbuffer);
	RHIRenderPassDesc D = RHIRenderPassDesc(gbuffer, ERenderPassLoadOp::Clear);
	D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	List->BeginRenderPass(D);
	SceneRender->RenderScene(List, false, gbuffer, false, eyeindex);
	List->EndRenderPass();
	gbuffer->MakeReadyForComputeUse(List);
	//List->SetRenderTarget(nullptr);
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
	//DebugList->SetRenderTarget(DDOs[DebugList->GetDeviceIndex()].MainFrameBuffer);
	//DebugList->ClearFrameBuffer(DDOs[DebugList->GetDeviceIndex()].MainFrameBuffer);
	DebugList->BeginRenderPass(RHIRenderPassDesc(DDOs[DebugList->GetDeviceIndex()].MainFrameBuffer, ERenderPassLoadOp::Clear));
	int currentDebugType = RHI::GetRenderSettings()->GetDebugRenderMode();
	int VisAlpha = 0;

	if (currentDebugType == ERenderDebugOutput::GBuffer_RoughNess)
	{
		DebugList->SetFrameBufferTexture(DDOs[DebugList->GetDeviceIndex()].Gbuffer, 0, 2);
		VisAlpha = 1;
	}
	else if (currentDebugType == ERenderDebugOutput::GBuffer_Metallic)
	{
		DebugList->SetFrameBufferTexture(DDOs[DebugList->GetDeviceIndex()].Gbuffer, 0, 1);
		VisAlpha = 1;
	}
	else
	{
		DebugList->SetFrameBufferTexture(DDOs[DebugList->GetDeviceIndex()].Gbuffer, 0, currentDebugType - 1);
	}
	DebugList->SetRootConstant(1, 1, &VisAlpha, 0);
	DDOs[DebugList->GetDeviceIndex()].DeferredShader->RenderScreenQuad(DebugList);
	DebugList->EndRenderPass();
	DDOs[DebugList->GetDeviceIndex()].Gbuffer->MakeReadyForComputeUse(DebugList, true);
	DebugList->Execute();
}
#endif
void DeferredRenderer::LightingPass(RHICommandList* List, FrameBuffer* GBuffer, FrameBuffer* output, int eyeindex)
{
	DeviceDependentObjects* Object = &DDOs[List->GetDeviceIndex()];
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = Object->DeferredShader;
	desc.FrameBufferTarget = Object->MainFrameBuffer;
	List->SetPipelineStateDesc(desc);

	RHIRenderPassDesc D = RHIRenderPassDesc(output, ERenderPassLoadOp::Clear);
#if NOSHADOW
	D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
#endif
	List->BeginRenderPass(D);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::PosTex, 0);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::NormalTex, 1);
	List->SetFrameBufferTexture(GBuffer, DeferredLightingShaderRSBinds::AlbedoTex, 2);

	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		if (RHI::GetRenderSettings()->GetRTSettings().UseForReflections)
		{
			List->SetFrameBufferTexture(Object->RTBuffer, DeferredLightingShaderRSBinds::ScreenSpecular);
		}

		List->SetTexture(MainScene->GetLightingData()->SkyBox, DeferredLightingShaderRSBinds::SpecBlurMap);

		//List->SetFrameBufferTexture(SceneRender->probes[0]->CapturedTexture, DeferredLightingShaderRSBinds::SpecBlurMap);
	}
	List->SetFrameBufferTexture(DDOs[List->GetDeviceIndex()].ConvShader->CubeBuffer, DeferredLightingShaderRSBinds::DiffuseIr);
	List->SetFrameBufferTexture(DDOs[List->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, DeferredLightingShaderRSBinds::EnvBRDF);

	SceneRender->BindLightsBuffer(List, DeferredLightingShaderRSBinds::LightDataCBV);
	SceneRender->BindMvBuffer(List, DeferredLightingShaderRSBinds::MVCBV, eyeindex);
#if !NOSHADOW
	mShadowRenderer->BindShadowMapsToTextures(List);
#endif
	DDOs[List->GetDeviceIndex()].DeferredShader->RenderScreenQuad(List);

	//transparent pass
	if (RHI::GetRenderSettings()->GetSettingsForRender().EnableTransparency)
	{
		GBuffer->BindDepthWithColourPassthrough(List, output);
		SceneRender->SetupBindsForForwardPass(List, eyeindex);
		SceneRender->Controller->RenderPass(ERenderPass::TransparentPass, List);
	}
	List->EndRenderPass();

#if !NOSHADOW
	mShadowRenderer->Unbind(List);
#endif
	//	List->SetRenderTarget(nullptr);
	if (List->GetDeviceIndex() == 0)
	{
		DDOs[0].MainFrameBuffer->MakeReadyForCopy(List);
	}
#if !NOSHADOW
	RenderSkybox(List, output, GBuffer);
#endif
	GBuffer->MakeReadyForComputeUse(List, true);
	if (Object->RTBuffer)
	{
		Object->RTBuffer->MakeReadyForComputeUse(List);
	}
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


void DeferredRenderer::OnStaticUpdate()
{}

FrameBuffer* DeferredRenderer::GetGBuffer()
{
	return DDOs[0].Gbuffer;
}
