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

#define CUBEMAPS 0
ForwardRenderer::ForwardRenderer(int width, int height) :RenderEngine(width, height)
{

}

void ForwardRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	FilterBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	if (DeviceObjects[1].FrameBuffer != nullptr)
	{
		DeviceObjects[1].FrameBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	}
	HandleCameraResize();
	RenderEngine::Resize(width, height);
}

ForwardRenderer::~ForwardRenderer()
{}

void ForwardRenderer::OnRender()
{
	ShadowPass();
	CubeMapPass();
	for (int i = 0; i < DevicesInUse; i++)
	{
		MainPass(DeviceObjects[i].MainCommandList);
	}
	RenderSkybox();
	if (DevicesInUse > 1)
	{
		DeviceObjects[1].FrameBuffer->ResolveSFR(FilterBuffer);
	}
	PostProcessPass();
}

void ForwardRenderer::PostInit()
{

	for (int i = 0; i < DevicesInUse; i++)
	{
		SetupOnDevice(RHI::GetDeviceContext(i));
	}
#if DEBUG_CUBEMAPS
	SkyBox->test = Conv->CubeBuffer;
	SkyBox->test = probes[0]->CapturedTexture;
#endif
	//probes.push_back(new RelfectionProbe());
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
	DeviceObjects[TargetDevice->GetDeviceIndex()].FrameBuffer = RHI::CreateFrameBuffer(TargetDevice, Desc);
	DDOs[TargetDevice->GetDeviceIndex()].SkyboxShader = new Shader_Skybox(TargetDevice);
	DDOs[TargetDevice->GetDeviceIndex()].SkyboxShader->Init(DeviceObjects[TargetDevice->GetDeviceIndex()].FrameBuffer, nullptr);
	DeviceObjects[TargetDevice->GetDeviceIndex()].MainCommandList = RHI::CreateCommandList(ECommandListType::Graphics, TargetDevice);
	RHIPipeLineStateDesc desc;
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = DeviceObjects[TargetDevice->GetDeviceIndex()].FrameBuffer;
	DeviceObjects[TargetDevice->GetDeviceIndex()].MainCommandList->SetPipelineStateDesc(desc);
	NAME_RHI_OBJECT(DeviceObjects[TargetDevice->GetDeviceIndex()].MainCommandList);
	if (TargetDevice->GetDeviceIndex() == 0)
	{
		FilterBuffer = DeviceObjects[TargetDevice->GetDeviceIndex()].FrameBuffer;
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
	CubemapCaptureList->SetFrameBufferTexture(Conv->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		CubemapCaptureList->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}
	CubemapCaptureList->SetFrameBufferTexture(envMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);
	SceneRender->UpdateRelflectionProbes(probes, CubemapCaptureList);

	CubemapCaptureList->Execute();
#endif
}

void ForwardRenderer::MainPass(RHICommandList* Cmdlist)
{
	Cmdlist->ResetList();
	//if (Cmdlist->GetDeviceIndex() == 1)
	//{
	//	Cmdlist->GetDevice()->GetTimeManager()->StartTotalGPUTimer(Cmdlist);
	//}
	Cmdlist->GetDevice()->GetTimeManager()->StartTimer(Cmdlist, EGPUTIMERS::MainPass);
	if (Cmdlist->GetDeviceIndex() == 0)
	{
		Cmdlist->SetScreenBackBufferAsRT();
		Cmdlist->ClearScreen();
	}
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->BindShadowMapsToTextures(Cmdlist);
	}
	Cmdlist->SetRenderTarget(DeviceObjects[Cmdlist->GetDeviceIndex()].FrameBuffer);
	Cmdlist->ClearFrameBuffer(DeviceObjects[Cmdlist->GetDeviceIndex()].FrameBuffer);
	if (RHI::GetMGPUSettings()->SplitShadowWork || RHI::GetMGPUSettings()->SFRSplitShadows)
	{
		glm::ivec2 Res = glm::ivec2(GetScaledWidth(), GetScaledHeight());
		Cmdlist->SetRootConstant(MainShaderRSBinds::ResolutionCBV, 2, &Res, 0);
	}
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		Cmdlist->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}
	Cmdlist->SetFrameBufferTexture(DDOs[Cmdlist->GetDeviceIndex()].ConvShader->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	Cmdlist->SetFrameBufferTexture(DDOs[Cmdlist->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);

	SceneRender->UpdateMV(MainCamera);
	SceneRender->RenderScene(Cmdlist, false, DeviceObjects[Cmdlist->GetDeviceIndex()].FrameBuffer);

	Cmdlist->SetRenderTarget(nullptr);
	Cmdlist->GetDevice()->GetTimeManager()->EndTimer(Cmdlist, EGPUTIMERS::MainPass);
	mShadowRenderer->Unbind(Cmdlist);
	if (Cmdlist->GetDeviceIndex() == 0 && DevicesInUse > 1)
	{
		DeviceObjects[1].FrameBuffer->MakeReadyForCopy(Cmdlist);
	}
	FilterBuffer->MakeReadyForCopy(Cmdlist);
	Cmdlist->Execute();

	if (Cmdlist->GetDeviceIndex() == 0)
	{
		ParticleSystemManager::Get()->Render(FilterBuffer);
	}
}

void ForwardRenderer::RenderSkybox()
{
	if (DevicesInUse > 1)
	{
		DDOs[1].SkyboxShader->Render(SceneRender, DeviceObjects[1].FrameBuffer, nullptr);
	}
	DDOs[0].SkyboxShader->Render(SceneRender, FilterBuffer, nullptr);
}

void ForwardRenderer::DestoryRenderWindow()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		EnqueueSafeRHIRelease(DeviceObjects[i].MainCommandList);
		EnqueueSafeRHIRelease(DeviceObjects[i].FrameBuffer);
	}
	FilterBuffer = nullptr;
	EnqueueSafeRHIRelease(CubemapCaptureList);
}

void ForwardRenderer::FinaliseRender()
{

}

void ForwardRenderer::OnStaticUpdate()
{}

