#include "ForwardRenderer.h"
#include "RHI/RHI.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "Core/Engine.h"
#include "RHI/DeviceContext.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/ParticleSystemManager.h"
#include "Rendering/Core/RelfectionProbe.h"
#include "Editor/EditorWindow.h"
#include "Editor/EditorCore.h"
#include "Rendering/Shaders/Generation/Shader_Convolution.h"
#include "Rendering/Shaders/Generation/Shader_EnvMap.h"
#include "RHI/SFRController.h"
#include "../Core/Mesh/MeshPipelineController.h"
#include "../Core/Shader_PreZ.h"
#include "../VR/VRCamera.h"
#include "../VR/HMD.h"

#define CUBEMAPS 1
ForwardRenderer::ForwardRenderer(int width, int height) :RenderEngine(width, height)
{

}

void ForwardRenderer::PreZPass(RHICommandList* Cmdlist)
{
	Cmdlist->StartTimer(EGPUTIMERS::PreZ);
	RHIPipeLineStateDesc desc;
	desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_PreZ>(), FilterBuffer);
	desc.DepthCompareFunction = COMPARISON_FUNC_LESS;
	desc.DepthStencilState.DepthWrite = true;
	Cmdlist->SetPipelineStateDesc(desc);
	Cmdlist->SetRenderTarget(DeviceObjects[Cmdlist->GetDeviceIndex()].MainFrameBuffer);
	Cmdlist->ClearFrameBuffer(DeviceObjects[Cmdlist->GetDeviceIndex()].MainFrameBuffer);
	SceneRender->RenderScene(Cmdlist, true, FilterBuffer);
	Cmdlist->EndTimer(EGPUTIMERS::PreZ);
}

void ForwardRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	FilterBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	if (DeviceObjects[1].MainFrameBuffer != nullptr)
	{
		DeviceObjects[1].MainFrameBuffer->Resize(GetScaledWidth(), GetScaledHeight());
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
		DeviceObjects[1].MainFrameBuffer->ResolveSFR(FilterBuffer);
	}

	PostProcessPass();
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
	DeviceObjects[TargetDevice->GetDeviceIndex()].MainFrameBuffer = RHI::CreateFrameBuffer(TargetDevice, Desc);
	DDOs[TargetDevice->GetDeviceIndex()].SkyboxShader = new Shader_Skybox(TargetDevice);
	DDOs[TargetDevice->GetDeviceIndex()].SkyboxShader->Init(DeviceObjects[TargetDevice->GetDeviceIndex()].MainFrameBuffer, nullptr);
	DeviceObjects[TargetDevice->GetDeviceIndex()].MainCommandList = RHI::CreateCommandList(ECommandListType::Graphics, TargetDevice);
	RHIPipeLineStateDesc desc;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = DeviceObjects[TargetDevice->GetDeviceIndex()].MainFrameBuffer;
	DeviceObjects[TargetDevice->GetDeviceIndex()].MainCommandList->SetPipelineStateDesc(desc);
	NAME_RHI_OBJECT(DeviceObjects[TargetDevice->GetDeviceIndex()].MainCommandList);
	if (TargetDevice->GetDeviceIndex() == 0)
	{
		FilterBuffer = DeviceObjects[TargetDevice->GetDeviceIndex()].MainFrameBuffer;
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
	if (RHI::SupportVR() && RHI::GetHMD() != nullptr)
	{
		VRCamera* VRCam = RHI::GetHMD()->GetVRCamera();
		SceneRender->UpdateMV(VRCam);
		DeviceObjects[0].MainCommandList->ResetList();
		MainPass(DeviceObjects[0].MainCommandList, DeviceObjects[0].MainFrameBuffer, EEye::Left);
		//MainPass(DeviceObjects[0].MainCommandList, DeviceObjects[0].RightEyeFramebuffer, EEye::Right);
		DeviceObjects[0].MainCommandList->Execute();
		RenderSkybox();
	}
	else
	{
		SceneRender->UpdateMV(MainCamera);
		for (int i = 0; i < DevicesInUse; i++)
		{
			DeviceObjects[i].MainCommandList->ResetList();
			MainPass(DeviceObjects[i].MainCommandList, DeviceObjects[i].MainFrameBuffer);
			DeviceObjects[i].MainCommandList->Execute();
		}
		RenderSkybox();
	}
}

void ForwardRenderer::MainPass(RHICommandList* Cmdlist, FrameBuffer* targetbuffer, int index)
{
	const bool PREZ = RHI::GetRenderSettings()->UseZPrePass;
	if (PREZ)
	{
		PreZPass(Cmdlist);
	}
	Cmdlist->GetDevice()->GetTimeManager()->StartTimer(Cmdlist, EGPUTIMERS::MainPass);
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader(), DeviceObjects[Cmdlist->GetDeviceIndex()].MainFrameBuffer);
	Cmdlist->SetPipelineStateDesc(desc);
	if (Cmdlist->GetDeviceIndex() == 0)
	{
		Cmdlist->SetScreenBackBufferAsRT();
		Cmdlist->ClearScreen();
	}
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


	SceneRender->RenderScene(Cmdlist, false, targetbuffer, index);
	//render the transparent objects AFTER the main scene
	SceneRender->Controller->RenderPass(ERenderPass::TransparentPass, Cmdlist);
	Cmdlist->SetRenderTarget(nullptr);
	Cmdlist->GetDevice()->GetTimeManager()->EndTimer(Cmdlist, EGPUTIMERS::MainPass);
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

void ForwardRenderer::RenderSkybox()
{
	if (DevicesInUse > 1)
	{
		DDOs[1].SkyboxShader->Render(SceneRender, DeviceObjects[1].MainFrameBuffer, nullptr);
	}
	DDOs[0].SkyboxShader->Render(SceneRender, FilterBuffer, nullptr);

}

void ForwardRenderer::DestoryRenderWindow()
{
	MemoryUtils::DeleteVector(probes);
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		EnqueueSafeRHIRelease(DeviceObjects[i].MainCommandList);
		EnqueueSafeRHIRelease(DeviceObjects[i].MainFrameBuffer);
	}
	FilterBuffer = nullptr;
	EnqueueSafeRHIRelease(CubemapCaptureList);
}

void ForwardRenderer::FinaliseRender()
{

}

void ForwardRenderer::OnStaticUpdate()
{}

