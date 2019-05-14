#include "ForwardRenderer.h"
#include "Rendering/Core/Mesh/MeshPipelineController.h"
#include "Rendering/Core/ParticleSystemManager.h"
#include "Rendering/Core/RelfectionProbe.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/Shader_PreZ.h"
#include "Rendering/Shaders/Generation/Shader_EnvMap.h"
#include "Rendering/VR/HMD.h"
#include "RHI/DeviceContext.h"
#include "../Core/Culling/CullingManager.h"
#include "RHI/RHI.h"
#include "../VR/VRCamera.h"

#define CUBEMAPS 0
ForwardRenderer::ForwardRenderer(int width, int height) :RenderEngine(width, height)
{

}

void ForwardRenderer::PreZPass(RHICommandList* Cmdlist, FrameBuffer* target, int eyeindex)
{
	Cmdlist->StartTimer(EGPUTIMERS::PreZ);
	RHIPipeLineStateDesc desc;
	desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_PreZ>(), FilterBuffer);
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
	FilterBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	if (DDOs[1].MainFrameBuffer != nullptr)
	{
		DDOs[1].MainFrameBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	}
	HandleCameraResize();
	RenderEngine::Resize(width, height);
}

ForwardRenderer::~ForwardRenderer()
{

}

void ForwardRenderer::OnRender()
{
	ShadowPass();
	CubeMapPass();
	RunMainPass();
	ParticleSystemManager::Get()->Render(FilterBuffer);
	if (DevicesInUse > 1)
	{
		DDOs[1].MainFrameBuffer->ResolveSFR(FilterBuffer);
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
	probes.push_back(new RelfectionProbe());
#if DEBUG_CUBEMAPS
	//SkyBox->test = Conv->CubeBuffer;
	DDOs[0].SkyboxShader->test = probes[0]->CapturedTexture;
#endif	
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
	DDOs[TargetDevice->GetDeviceIndex()].MainFrameBuffer = RHI::CreateFrameBuffer(TargetDevice, Desc);
	if (RHI::SupportVR())
	{
		DDOs[TargetDevice->GetDeviceIndex()].RightEyeFramebuffer = RHI::CreateFrameBuffer(TargetDevice, Desc);
	}
	DDOs[TargetDevice->GetDeviceIndex()].SkyboxShader = new Shader_Skybox(TargetDevice);
	DDOs[TargetDevice->GetDeviceIndex()].SkyboxShader->Init(DDOs[TargetDevice->GetDeviceIndex()].MainFrameBuffer, nullptr);
	DDOs[TargetDevice->GetDeviceIndex()].MainCommandList = RHI::CreateCommandList(ECommandListType::Graphics, TargetDevice);
	RHIPipeLineStateDesc desc;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = DDOs[TargetDevice->GetDeviceIndex()].MainFrameBuffer;
	DDOs[TargetDevice->GetDeviceIndex()].MainCommandList->SetPipelineStateDesc(desc);
	NAME_RHI_OBJECT(DDOs[TargetDevice->GetDeviceIndex()].MainCommandList);
	if (TargetDevice->GetDeviceIndex() == 0)
	{
		FilterBuffer = DDOs[TargetDevice->GetDeviceIndex()].MainFrameBuffer;
	}
#if CUBEMAPS
	/*VKanRHI::Get()->thelist = MainCommandList;*/
	CubemapCaptureList = RHI::CreateCommandList(ECommandListType::Graphics, TargetDevice);
	desc = RHIPipeLineStateDesc();
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = FilterBuffer;
	CubemapCaptureList->SetPipelineStateDesc(desc);
#endif
}

void ForwardRenderer::CubeMapPass()
{
#if CUBEMAPS
	CubemapCaptureList->ResetList();
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->BindShadowMapsToTextures(CubemapCaptureList);
	}
	CubemapCaptureList->SetFrameBufferTexture(DDOs[CubemapCaptureList->GetDeviceIndex()].ConvShader->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		CubemapCaptureList->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}
	CubemapCaptureList->SetFrameBufferTexture(DDOs[CubemapCaptureList->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);
	SceneRender->UpdateRelflectionProbes(probes, CubemapCaptureList);

	CubemapCaptureList->Execute();
#endif
}

void ForwardRenderer::RunMainPass()
{
	UpdateMVForMainPass();
	if (RHI::SupportVR() && RHI::GetHMD() != nullptr)
	{
		DDOs[0].MainCommandList->ResetList();
		DDOs[0].MainCommandList->StartTimer(EGPUTIMERS::MainPass);
		Culling->UpdateMainPassFrustumCulling(RHI::GetHMD()->GetVRCamera()->GetEyeCam(EEye::Left), MainScene);
		MainPass(DDOs[0].MainCommandList, DDOs[0].MainFrameBuffer, EEye::Left);
		Culling->UpdateMainPassFrustumCulling(RHI::GetHMD()->GetVRCamera()->GetEyeCam(EEye::Right), MainScene);
		MainPass(DDOs[0].MainCommandList, DDOs[0].RightEyeFramebuffer, EEye::Right);
		DDOs[0].MainCommandList->EndTimer(EGPUTIMERS::MainPass);
		RenderSkybox(&DDOs[0]);
		DDOs[0].MainCommandList->Execute();
	}
	else
	{
		for (int i = 0; i < DevicesInUse; i++)
		{
			DDOs[i].MainCommandList->ResetList();
			MainPass(DDOs[i].MainCommandList, DDOs[i].MainFrameBuffer);
			RenderSkybox(&DDOs[i]);
			DDOs[i].MainCommandList->Execute();
		}

	}
}

void ForwardRenderer::MainPass(RHICommandList* Cmdlist, FrameBuffer* targetbuffer, int index)
{
	const bool PREZ = RHI::GetRenderSettings()->IsUsingZPrePass();
	if (PREZ)
	{
		PreZPass(Cmdlist, targetbuffer, index);
	}

	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader(), DDOs[Cmdlist->GetDeviceIndex()].MainFrameBuffer);
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
	if (RHI::GetMGPUSettings()->SplitShadowWork || RHI::GetMGPUSettings()->SFRSplitShadows)
	{
		glm::ivec2 Res = glm::ivec2(GetScaledWidth(), GetScaledHeight());
		Cmdlist->SetRootConstant(MainShaderRSBinds::ResolutionCBV, 2, &Res, 0);
	}
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
#if CUBEMAPS
		Cmdlist->SetFrameBufferTexture(probes[0]->CapturedTexture, MainShaderRSBinds::SpecBlurMap);
#else
		Cmdlist->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
#endif
	}
	Cmdlist->SetFrameBufferTexture(DDOs[Cmdlist->GetDeviceIndex()].ConvShader->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	Cmdlist->SetFrameBufferTexture(DDOs[Cmdlist->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);


	SceneRender->RenderScene(Cmdlist, false, targetbuffer, false, index);
	//render the transparent objects AFTER the main scene
	SceneRender->Controller->RenderPass(ERenderPass::TransparentPass, Cmdlist);
	Cmdlist->SetRenderTarget(nullptr);
#if !BASIC_RENDER_ONLY
	mShadowRenderer->Unbind(Cmdlist);
#endif
	if (Cmdlist->GetDeviceIndex() == 0 && DevicesInUse > 1)
	{
		//#TODO check
		targetbuffer->MakeReadyForCopy(Cmdlist);
	}
	targetbuffer->MakeReadyForCopy(Cmdlist);

}

void ForwardRenderer::RenderSkybox(DeviceDependentObjects* Object)
{
	Object->SkyboxShader->Render(SceneRender, Object->MainCommandList, Object->MainFrameBuffer, nullptr);
	if (RHI::IsRenderingVR())
	{
		Object->SkyboxShader->Render(SceneRender, Object->MainCommandList, Object->RightEyeFramebuffer, nullptr);
	}
}

void ForwardRenderer::DestoryRenderWindow()
{
	MemoryUtils::DeleteVector(probes);
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		EnqueueSafeRHIRelease(DDOs[i].MainCommandList);
		EnqueueSafeRHIRelease(DDOs[i].MainFrameBuffer);
	}
	FilterBuffer = nullptr;
	EnqueueSafeRHIRelease(CubemapCaptureList);
}

void ForwardRenderer::FinaliseRender()
{

}

void ForwardRenderer::OnStaticUpdate()
{}

