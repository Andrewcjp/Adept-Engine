#include "DeferredRenderer.h"
#include "../Rendering/Core/Mesh.h"
#include "Core/Assets/ImageLoader.h"
#include "RHI/RHI.h"
#include "../Core/Components/MeshRendererComponent.h"
#include "../Core/Assets/Scene.h"
#include "../Rendering/PostProcessing/PostProcessing.h"
#include "../RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "../Editor/Editor_Camera.h"
void DeferredRenderer::Render()
{
//////	/*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
//	shadower->RenderShadowMaps(MainCamera, Lights, Objects);
#if WITH_EDITOR
//todo!
	if (EditorCam != nullptr && EditorCam->GetEnabled())
	{
		if (MainCamera != EditorCam->GetCamera())
		{
			MainCamera = EditorCam->GetCamera();
		}
	}
	else
	{
		/*if (MainCamera == EditorCam->GetCamera())
		{
		MainCamera = mainscene->GetCurrentRenderCamera();
		}*/
	}
#endif




	GeometryPass();

	LightingPass();
	//RenderFitlerBufferOutput();
	Post->ExecPPStack(OutputBuffer);
}

void DeferredRenderer::Init()
{
	GPUStateCache::Create();
	MainCamera = new Camera(glm::vec3(0, 2, 0), 90.0f, static_cast<float>(m_width / m_height), 0.1f, 1000.0f);
	DeferredShader = new Shader_Deferred();
	shadower = new ShadowRenderer();
	Post = new PostProcessing();
	Post->Init();
	FilterBuffer = RHI::CreateFrameBuffer(m_width, m_height);
	outshader = new ShaderOutput(FilterBuffer->GetWidth(), FilterBuffer->GetHeight());
	GFrameBuffer = RHI::CreateFrameBuffer(m_width, m_height, RHI::GetDeviceContext(0), 1.0f, FrameBuffer::GBuffer); 
	OutputBuffer = RHI::CreateFrameBuffer(m_width, m_height, RHI::GetDeviceContext(0), 1.0f, FrameBuffer::ColourDepth);
	WriteList = RHI::CreateCommandList(RHI::GetDeviceContext(0));
	MainShader = new Shader_Main(false);
	WriteList->CreatePipelineState(MainShader, GFrameBuffer);
	LightingList = RHI::CreateCommandList(RHI::GetDeviceContext(0));	
	LightingList->SetPipelineState(PipeLineState{ false,false,false });
	LightingList->CreatePipelineState(DeferredShader);
	skybox = new GameObject();
	D3D12RHI::Instance->AddLinkedFrameBuffer(GFrameBuffer);
	D3D12RHI::Instance->AddLinkedFrameBuffer(OutputBuffer);	

}

DeferredRenderer::~DeferredRenderer()
{
}

Camera * DeferredRenderer::GetMainCam()
{
	return MainCamera;
}

void DeferredRenderer::AddGo(GameObject * g)
{
	Objects.push_back(g);
}


void DeferredRenderer::AddLight(Light * l)
{
	Lights.push_back(l);
}

void DeferredRenderer::PrepareData()
{
	for (size_t i = 0; i < (*mainscene->GetObjects()).size(); i++)
	{
		MainShader->UpdateUnformBufferEntry(MainShader->CreateUnformBufferEntry((*mainscene->GetObjects())[i]->GetTransform()), (int)i);
	}
}

void DeferredRenderer::GeometryPass()
{	
	if (RHI::GetType() == RenderSystemD3D12)
	{
		if (once)
		{
			D3D12RHI::Instance->ExecSetUpList();
			once = false;
		}
	}

	if (mainscene->StaticSceneNeedsUpdate)
	{
		MainShader->UpdateLightBuffer(*mainscene->GetLights());
		PrepareData();
		MainShader->UpdateCBV();
	}

	WriteList->ResetList();
	WriteList->SetRenderTarget(GFrameBuffer);
	WriteList->ClearFrameBuffer(GFrameBuffer);
	MainShader->UpdateMV(MainCamera);
	MainShader->BindLightsBuffer(WriteList);
	MainShader->UpdateMV(MainCamera);
	for (size_t i = 0; i < (*mainscene->GetObjects()).size(); i++)
	{
		MainShader->SetActiveIndex(WriteList, i);
		(*mainscene->GetObjects())[i]->Render(false, WriteList);
	}
	WriteList->SetRenderTarget(nullptr);
	WriteList->Execute();
}
void DeferredRenderer::RenderSkybox(bool ismain)
{

}
void DeferredRenderer::SSAOPass()
{
	//SSAOBuffer->BindBufferAsRenderTarget();
	//glClear(GL_COLOR_BUFFER_BIT);
	//->BindToTextureUnit();
	SSAOShader->SetShaderActive();
	SSAOShader->UpdateUniforms(nullptr, MainCamera, Lights);
	SSAOShader->RenderPlane();
	//SSAOBuffer->UnBind();
}
void DeferredRenderer::LightingPass()
{
	LightingList->ResetList();
	LightingList->ClearScreen();

	LightingList->SetRenderTarget(OutputBuffer);
	LightingList->ClearFrameBuffer(OutputBuffer);
	LightingList->SetFrameBufferTexture(GFrameBuffer, 0, 0);
	LightingList->SetFrameBufferTexture(GFrameBuffer, 1, 1);
	LightingList->SetFrameBufferTexture(GFrameBuffer, 3, 2);
	MainShader->BindLightsBuffer(LightingList,true);
	DeferredShader->RenderScreenQuad(LightingList);
	LightingList->SetRenderTarget(nullptr);
	LightingList->Execute();
}

void DeferredRenderer::ShadowPass()
{

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

Shader * DeferredRenderer::GetMainShader()
{
	return MainShader;
}


void DeferredRenderer::DestoryRenderWindow()
{
}

void DeferredRenderer::FinaliseRender()
{
}
