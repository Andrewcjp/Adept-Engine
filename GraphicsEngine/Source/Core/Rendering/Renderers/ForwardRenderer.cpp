#include "ForwardRenderer.h"
#include "RHI/RHI.h"
#include "Core/Components/MeshRendererComponent.h"

#include "Rendering/PostProcessing/PostProcessing.h"
#include "Core/Engine.h"
#include "RHI/DeviceContext.h"

ForwardRenderer::ForwardRenderer(int width, int height) :RenderEngine(width, height)
{}

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
#endif

}

void ForwardRenderer::SetupOnDevice(DeviceContext* TargetDevice)
{
	MainShader = new Shader_Main();
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateColourDepth(GetScaledWidth(), GetScaledHeight());
	Desc.AllowUnordedAccess = true;
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	FilterBuffer = RHI::CreateFrameBuffer(TargetDevice, Desc);
	if (MainScene == nullptr)
	{
		MainScene = new Scene();
	}
	SkyBox = new Shader_Skybox();
	SkyBox->Init(FilterBuffer, nullptr);
	MainCommandList = RHI::CreateCommandList(ECommandListType::Graphics, TargetDevice);
	//finally init the pipeline!
	MainCommandList->CreatePipelineState(MainShader, FilterBuffer);
}


void ForwardRenderer::MainPass()
{
	MainCommandList->ResetList();

	MainCommandList->GetDevice()->GetTimeManager()->StartTotalGPUTimer(MainCommandList);
	MainCommandList->GetDevice()->GetTimeManager()->StartTimer(MainCommandList, EGPUTIMERS::MainPass);
	MainCommandList->SetScreenBackBufferAsRT();
	MainCommandList->ClearScreen();
	MainShader->UpdateMV(MainCamera);
	MainShader->BindLightsBuffer(MainCommandList);

	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->BindShadowMapsToTextures(MainCommandList);
	}
	MainShader->UpdateMV(MainCamera);
	MainCommandList->SetRenderTarget(FilterBuffer);
	MainCommandList->ClearFrameBuffer(FilterBuffer);
	MainCommandList->SetFrameBufferTexture(Conv->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		MainCommandList->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}
	MainCommandList->SetFrameBufferTexture(envMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);
	for (size_t i = 0; i < (*MainScene->GetRenderableObjects()).size(); i++)
	{
		GameObject* CurrentObj = (*MainScene->GetRenderableObjects())[i];
		MainShader->SetActiveIndex(MainCommandList, (int)i);
		if (CurrentObj->GetMat()->GetProperties()->ShaderInUse != nullptr)
		{
			MainCommandList->SetPipelineStateObject(CurrentObj->GetMat()->GetProperties()->ShaderInUse, FilterBuffer);
		}
		else
		{
			MainCommandList->SetPipelineStateObject(MainShader, FilterBuffer);
		}
		CurrentObj->Render(false, MainCommandList);
	}
	MainCommandList->SetRenderTarget(nullptr);
	MainCommandList->GetDevice()->GetTimeManager()->EndTimer(MainCommandList, EGPUTIMERS::MainPass);
	mShadowRenderer->Unbind(MainCommandList);
	MainCommandList->Execute();
}

void ForwardRenderer::RenderSkybox()
{
	SkyBox->Render(MainShader, FilterBuffer, nullptr);
}

void ForwardRenderer::DestoryRenderWindow()
{
	delete MainCommandList;
}

void ForwardRenderer::FinaliseRender()
{

}

void ForwardRenderer::OnStaticUpdate()
{}

