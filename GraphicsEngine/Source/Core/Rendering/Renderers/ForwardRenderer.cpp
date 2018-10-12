#include "ForwardRenderer.h"
#include "RHI/RHI.h"
#include "Core/Components/MeshRendererComponent.h"

#include "Rendering/PostProcessing/PostProcessing.h"
#include "Core/Engine.h"
#include "RHI/DeviceContext.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/ParticleSystemManager.h"
#include "Rendering/Core/RelfectionProbe.h"
ForwardRenderer::ForwardRenderer(int width, int height) :RenderEngine(width, height)
{

}

void ForwardRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	FilterBuffer->Resize(GetScaledWidth(), GetScaledHeight());
	if (MainCamera != nullptr)
	{
		MainCamera->UpdateProjection((float)GetScaledWidth() / (float)GetScaledHeight());
	}
	RenderEngine::Resize(width, height);

}

ForwardRenderer::~ForwardRenderer()
{}

void ForwardRenderer::OnRender()
{
	ShadowPass();
	CubeMapPass();
	MainPass();
	RenderSkybox();
	PostProcessPass();
}

#include "Rendering/Shaders/Generation/Shader_Convolution.h"
#include "Rendering/Shaders/Generation/Shader_EnvMap.h"
void ForwardRenderer::PostInit()
{
	SetupOnDevice(RHI::GetDeviceContext(0));
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
	FilterBuffer = RHI::CreateFrameBuffer(TargetDevice, Desc);
	SkyBox = ShaderComplier::GetShader<Shader_Skybox>();
	SkyBox->Init(FilterBuffer, nullptr);
	MainCommandList = RHI::CreateCommandList(ECommandListType::Graphics, TargetDevice);
	//finally init the pipeline!
	MainCommandList->CreatePipelineState(Material::GetDefaultMaterialShader(), FilterBuffer);
	NAME_RHI_OBJECT(MainCommandList);
	/*VKanRHI::Get()->thelist = MainCommandList;*/
	CubemapCaptureList = RHI::CreateCommandList(ECommandListType::Graphics, TargetDevice);
	//finally init the pipeline!
	CubemapCaptureList->CreatePipelineState(Material::GetDefaultMaterialShader(), FilterBuffer);

}
void ForwardRenderer::CubeMapPass()
{
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
}
void ForwardRenderer::MainPass()
{
	MainCommandList->ResetList();

	MainCommandList->GetDevice()->GetTimeManager()->StartTotalGPUTimer(MainCommandList);
	MainCommandList->GetDevice()->GetTimeManager()->StartTimer(MainCommandList, EGPUTIMERS::MainPass);
	MainCommandList->SetScreenBackBufferAsRT();
	MainCommandList->ClearScreen();
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->BindShadowMapsToTextures(MainCommandList);
	}
	MainCommandList->SetRenderTarget(FilterBuffer);
	MainCommandList->ClearFrameBuffer(FilterBuffer);
	MainCommandList->SetFrameBufferTexture(Conv->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		MainCommandList->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}
	MainCommandList->SetFrameBufferTexture(envMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);


	SceneRender->UpdateMV(MainCamera);
	SceneRender->RenderScene(MainCommandList, false, FilterBuffer);

	MainCommandList->SetRenderTarget(nullptr);
	MainCommandList->GetDevice()->GetTimeManager()->EndTimer(MainCommandList, EGPUTIMERS::MainPass);
	mShadowRenderer->Unbind(MainCommandList);
	MainCommandList->Execute();
	ParticleSystemManager::Get()->Render(FilterBuffer);
}

void ForwardRenderer::RenderSkybox()
{
	SkyBox->Render(SceneRender, FilterBuffer, nullptr);
}

void ForwardRenderer::DestoryRenderWindow()
{
	EnqueueSafeRHIRelease(MainCommandList);
	EnqueueSafeRHIRelease(CubemapCaptureList);
}

void ForwardRenderer::FinaliseRender()
{

}

void ForwardRenderer::OnStaticUpdate()
{}

