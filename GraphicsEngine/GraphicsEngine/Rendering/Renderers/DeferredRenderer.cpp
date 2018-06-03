#include "DeferredRenderer.h"
#include "../Rendering/Core/Mesh.h"
#include "RHI/RHI.h"
#include "../Core/Components/MeshRendererComponent.h"
#include "../Core/Assets/Scene.h"
#include "../Rendering/PostProcessing/PostProcessing.h"
#include "../RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "../Editor/Editor_Camera.h"
#include "../RHI/RenderAPIs/D3D12/D3D12TimeManager.h"
#include "Rendering\Shaders\Shader_Skybox.h"

#include "../RHI/DeviceContext.h"
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
	SkyBox->Render(MainShader, FilterBuffer, GFrameBuffer);
}

void DeferredRenderer::PostInit()
{
	MainShader = new Shader_Main(false);
	FilterBuffer = RHI::CreateFrameBuffer(m_width, m_height, RHI::GetDeviceContext(0), 1.0f, FrameBuffer::Colour);
	DeferredShader = new Shader_Deferred();
	GFrameBuffer = RHI::CreateFrameBuffer(m_width, m_height, RHI::GetDeviceContext(0), 1.0f, FrameBuffer::GBuffer);
	WriteList = RHI::CreateCommandList(RHI::GetDeviceContext(0));
	WriteList->CreatePipelineState(MainShader, GFrameBuffer);
	LightingList = RHI::CreateCommandList(RHI::GetDeviceContext(0));
	LightingList->SetPipelineState(PipeLineState{ false,false,false });
	LightingList->CreatePipelineState(DeferredShader);
	D3D12RHI::Instance->AddLinkedFrameBuffer(GFrameBuffer);
	SkyBox = new Shader_Skybox();
	SkyBox->Init(FilterBuffer, GFrameBuffer);
}


void DeferredRenderer::GeometryPass()
{
	if (MainScene->StaticSceneNeedsUpdate)
	{
		MainShader->UpdateLightBuffer(*MainScene->GetLights());
		PrepareData();
		MainShader->UpdateCBV();
	}
	 
	WriteList->ResetList();
	WriteList->GetDevice()->GetTimeManager()->StartTotalGPUTimer(WriteList);
	WriteList->SetRenderTarget(GFrameBuffer);
	WriteList->ClearFrameBuffer(GFrameBuffer);
	MainShader->BindLightsBuffer(WriteList);
	MainShader->UpdateMV(MainCamera);
	for (size_t i = 0; i < (*MainScene->GetObjects()).size(); i++)
	{
		MainShader->SetActiveIndex(WriteList, i);
		(*MainScene->GetObjects())[i]->Render(false, WriteList);
	}
	WriteList->SetRenderTarget(nullptr);
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
	LightingList->SetScreenBackBufferAsRT();
	LightingList->ClearScreen();

	LightingList->SetRenderTarget(FilterBuffer);
	LightingList->ClearFrameBuffer(FilterBuffer);
	LightingList->SetFrameBufferTexture(GFrameBuffer, 0, 0);
	LightingList->SetFrameBufferTexture(GFrameBuffer, 1, 1);
	LightingList->SetFrameBufferTexture(GFrameBuffer, 3, 2);
	MainShader->BindLightsBuffer(LightingList, true);
	DeferredShader->RenderScreenQuad(LightingList);
	LightingList->SetRenderTarget(nullptr);
	LightingList->Execute();
}

void DeferredRenderer::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	if (RHI::IsD3D12())
	{
		if (D3D12RHI::Instance)
		{
			D3D12RHI::Instance->ResizeSwapChain(width, height);
		}
	}
	if (MainCamera != nullptr)
	{
		MainCamera->UpdateProjection((float)width / (float)height);
	}
}

DeferredRenderer::~DeferredRenderer()
{}

void DeferredRenderer::DestoryRenderWindow()
{
	delete DeferredWriteShader;
	delete DeferredShader;
	delete GFrameBuffer;
	delete WriteList;
	delete LightingList;
	delete OutputBuffer;
}

void DeferredRenderer::FinaliseRender()
{}

void DeferredRenderer::OnStaticUpdate()
{}
