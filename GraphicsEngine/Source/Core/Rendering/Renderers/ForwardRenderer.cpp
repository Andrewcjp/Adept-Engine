#include "ForwardRenderer.h"
#include "Rendering/Core/Culling/CullingManager.h"
#include "Rendering/Core/Mesh/MeshPipelineController.h"
#include "Rendering/Core/ParticleSystemManager.h"
#include "Rendering/Core/RelfectionProbe.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/Shader_PreZ.h"
#include "Rendering/Shaders/Generation/Shader_EnvMap.h"
#include "Rendering/VR/HMD.h"
#include "Rendering/VR/VRCamera.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHI.h"
#include "../Core/LightCulling/LightCullingEngine.h"
#include "../RayTracing/RayTracingEngine.h"


ForwardRenderer::ForwardRenderer(int width, int height) :RenderEngine(width, height)
{

}

void ForwardRenderer::PreZPass(RHICommandList* Cmdlist, FrameBuffer* target, int eyeindex)
{
	Cmdlist->StartTimer(EGPUTIMERS::PreZ);
	RHIPipeLineStateDesc desc;
	desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_PreZ>(), DDOs[0].MainFrameBuffer);
	desc.DepthCompareFunction = COMPARISON_FUNC_LESS;
	desc.DepthStencilState.DepthWrite = true;
	Cmdlist->SetPipelineStateDesc(desc);
	Cmdlist->SetRenderTarget(target);
	Cmdlist->ClearFrameBuffer(target);
	SceneRender->RenderScene(Cmdlist, true, target, false, eyeindex);
	Cmdlist->EndTimer(EGPUTIMERS::PreZ);
}

void ForwardRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	DDOs[0].MainFrameBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	if (RHI::IsRenderingVR())
	{
		DDOs[0].RightEyeFramebuffer->Resize(GetScaledWidth(), GetScaledHeight());
	}
	if (DDOs[1].MainFrameBuffer != nullptr)
	{
		DDOs[1].MainFrameBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	}
	HandleCameraResize();
	RenderEngine::Resize(width, height);
}

FrameBuffer* ForwardRenderer::GetGBuffer()
{
	throw std::logic_error("The method or operation is not implemented.");
}

ForwardRenderer::~ForwardRenderer()
{

}

void ForwardRenderer::OnRender()
{
	RunLightCulling();
	ShadowPass();
	CubeMapPass();
	RenderOnDevice(RHI::GetDeviceContext(0));
	ParticleSystemManager::Get()->Render(&DDOs[0]);
	if (DevicesInUse > 1)
	{
		DDOs[1].MainFrameBuffer->ResolveSFR(DDOs[0].MainFrameBuffer);
	}
	PostProcessPass();
	PresentToScreen();

}


void ForwardRenderer::PostInit()
{
	for (int i = 0; i < DevicesInUse; i++)
	{
		SetupOnDevice(RHI::GetDeviceContext(i));
	}

}

void ForwardRenderer::SetupOnDevice(DeviceContext* TargetDevice)
{
	MainShader = new Shader_Main();
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(GetScaledWidth(), GetScaledHeight());
	Desc.AllowUnordedAccess = true;
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	Desc.IncludedInSFR = true;
	if (TargetDevice->GetDeviceIndex() > 0)
	{
		Desc.IsShared = true;
		Desc.DeviceToCopyTo = RHI::GetDeviceContext(0);
	}
	Desc.AllowDynamicResize = true;
	DDOs[TargetDevice->GetDeviceIndex()].MainFrameBuffer = RHI::CreateFrameBuffer(TargetDevice, Desc);
	if (RHI::SupportVR())
	{
		DDOs[TargetDevice->GetDeviceIndex()].RightEyeFramebuffer = RHI::CreateFrameBuffer(TargetDevice, Desc);
	}
	DDOs[TargetDevice->GetDeviceIndex()].SkyboxShader = new Shader_Skybox(TargetDevice);
	DDOs[TargetDevice->GetDeviceIndex()].SkyboxShader->Init(DDOs[TargetDevice->GetDeviceIndex()].MainFrameBuffer, nullptr);
	DDOs[TargetDevice->GetDeviceIndex()].MainCommandList[0] = RHI::CreateCommandList(ECommandListType::Graphics, TargetDevice);
	DDOs[TargetDevice->GetDeviceIndex()].MainCommandList[1] = RHI::CreateCommandList(ECommandListType::Graphics, TargetDevice);
	RHIPipeLineStateDesc desc;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = DDOs[TargetDevice->GetDeviceIndex()].MainFrameBuffer;
	DDOs[TargetDevice->GetDeviceIndex()].MainCommandList[0]->SetPipelineStateDesc(desc);
	NAME_RHI_OBJECT(DDOs[TargetDevice->GetDeviceIndex()].MainCommandList[0]);
	if (TargetDevice->GetDeviceIndex() == 0)
	{
		DDOs[0].MainFrameBuffer = DDOs[TargetDevice->GetDeviceIndex()].MainFrameBuffer;
	}
}

void ForwardRenderer::RenderOnDevice(DeviceContext * con)
{
	RunMainPass(&DDOs[con->GetDeviceIndex()], EEye::Left);
	if (RHI::IsRenderingVR())
	{
		RunMainPass(&DDOs[con->GetDeviceIndex()], EEye::Right);
	}
}

void ForwardRenderer::RunMainPass(DeviceDependentObjects* O, EEye::Type eye)
{
	RHICommandList* List = O->MainCommandList[eye];
	UpdateMVForMainPass();

	List->ResetList();
	List->StartTimer(EGPUTIMERS::MainPass);

	if (RHI::IsRenderingVR())
	{
		Culling->UpdateMainPassFrustumCulling(RHI::GetHMD()->GetVRCamera()->GetEyeCam(eye), MainScene);
	}
	else
	{
		Culling->UpdateMainPassFrustumCulling(MainCamera, MainScene);
	}
	MainPass(List, O->GetMain(eye), eye);
	List->EndTimer(EGPUTIMERS::MainPass);
	O->SkyboxShader->Render(SceneRender, List, O->GetMain(eye), nullptr);
	List->Execute();
	if (RHI::GetRenderSettings()->RaytracingEnabled())
	{
		RayTracingEngine::Get()->DispatchRays(O->GetMain(eye));
	}
}

void ForwardRenderer::MainPass(RHICommandList* Cmdlist, FrameBuffer* targetbuffer, int index)
{
	Cmdlist->GetDevice()->GetTimeManager()->StartTotalGPUTimer(Cmdlist);
	const bool PREZ = RHI::GetRenderSettings()->IsUsingZPrePass();
	if (PREZ)
	{
		PreZPass(Cmdlist, targetbuffer, index);
	}

	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader(), targetbuffer);
	Cmdlist->SetPipelineStateDesc(desc);

#if !BASIC_RENDER_ONLY
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->BindShadowMapsToTextures(Cmdlist);
	}
#endif
	Cmdlist->SetRenderTarget(targetbuffer);
	if (!PREZ)
	{
		Cmdlist->ClearFrameBuffer(targetbuffer);
	}
	//if (RHI::GetMGPUSettings()->SplitShadowWork || RHI::GetMGPUSettings()->SFRSplitShadows)
	{
		glm::ivec2 Res = glm::ivec2(GetScaledWidth(), GetScaledHeight());
		Cmdlist->SetRootConstant(MainShaderRSBinds::ResolutionCBV, 2, &Res, 0);
	}
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		//Cmdlist->SetFrameBufferTexture(SceneRender->probes[0]->CapturedTexture, MainShaderRSBinds::SpecBlurMap);
		Cmdlist->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}
	Cmdlist->SetFrameBufferTexture(DDOs[Cmdlist->GetDeviceIndex()].ConvShader->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	Cmdlist->SetFrameBufferTexture(DDOs[Cmdlist->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);
	LightCulling->BindLightBuffer(Cmdlist);
	SceneRender->RenderScene(Cmdlist, false, targetbuffer, false, index);
	//render the transparent objects AFTER the main scene
	SceneRender->Controller->RenderPass(ERenderPass::TransparentPass, Cmdlist);
	Cmdlist->SetRenderTarget(nullptr);
#if !BASIC_RENDER_ONLY
	mShadowRenderer->Unbind(Cmdlist);
#endif
	LightCulling->Unbind(Cmdlist);
	targetbuffer->MakeReadyForComputeUse(Cmdlist);

}


void ForwardRenderer::DestoryRenderWindow()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		//EnqueueSafeRHIRelease(DDOs[i].MainCommandList);
		EnqueueSafeRHIRelease(DDOs[i].MainFrameBuffer);
	}
	DDOs[0].MainFrameBuffer = nullptr;
	EnqueueSafeRHIRelease(CubemapCaptureList);
}

void ForwardRenderer::OnStaticUpdate()
{}

