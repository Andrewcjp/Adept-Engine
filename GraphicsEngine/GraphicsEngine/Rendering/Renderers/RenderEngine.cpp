#include "RenderEngine.h"
#include "../Rendering/Core/ShadowRenderer.h"
#include "../Core/Assets/Scene.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "../Rendering/PostProcessing/PostProcessing.h"
#include "../RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "../Editor/Editor_Camera.h"
#include "../Rendering/Shaders/Shader_Skybox.h"
RenderEngine::~RenderEngine()
{
	DestoryRenderWindow();
	delete mShadowRenderer;
	delete Post;
	delete MainShader;
	delete FilterBuffer;
}

void RenderEngine::Render()
{
	if (once)//todo: move me!
	{
		D3D12RHI::Instance->ExecSetUpList();
		once = false;
	}
	if (MainCamera == nullptr)
	{
		return;
	}
	if ((*MainScene->GetObjects()).size() == 0)
	{
		return;
	}
	PreRender();
	OnRender();
}

void RenderEngine::PreRender()
{
	if (MainScene->StaticSceneNeedsUpdate)
	{
		StaticUpdate();
	}
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
		if (MainCamera == EditorCam->GetCamera())
		{
			MainCamera = MainScene->GetCurrentRenderCamera();
		}
	}
#endif
}

//init common to both renderers
void RenderEngine::Init()
{
	mShadowRenderer = new ShadowRenderer();
	if (MainScene != nullptr)
	{
		mShadowRenderer->InitShadows(*MainScene->GetLights());
	}	
	Post = new PostProcessing();
	Post->Init();
	GPUStateCache::Create();
	PostInit();
	D3D12RHI::Instance->AddLinkedFrameBuffer(FilterBuffer);
}

void RenderEngine::PrepareData()
{
	for (size_t i = 0; i < (*MainScene->GetObjects()).size(); i++)
	{
		MainShader->UpdateUnformBufferEntry(MainShader->CreateUnformBufferEntry((*MainScene->GetObjects())[i]->GetTransform()), (int)i);
	}
}



void RenderEngine::StaticUpdate()
{
	mShadowRenderer->InitShadows(*MainScene->GetLights());
	mShadowRenderer->Renderered = false;
	MainShader->UpdateLightBuffer(*MainScene->GetLights());
	PrepareData();
	MainShader->UpdateCBV();

	OnStaticUpdate();
}

void RenderEngine::SetRenderSettings(RenderSettings set)
{
	CurrentRenderSettings = set;
}

void RenderEngine::SetScene(Scene * sc)
{
	MainScene = sc;
	if (sc == nullptr)
	{
		MainCamera = nullptr;
		mShadowRenderer->ClearShadowLights();
		return;
	}

	MainShader->RefreshLights();
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->InitShadows(*MainScene->GetLights());
		mShadowRenderer->Renderered = false;
	}
	if (sc == nullptr)
	{
		MainCamera = nullptr;
	}
	else
	{
		MainCamera = MainScene->GetCurrentRenderCamera();
	}
}

void RenderEngine::SetEditorCamera(Editor_Camera * cam)
{
	EditorCam = cam;
}

void RenderEngine::ShadowPass()
{	
	mShadowRenderer->RenderShadowMaps(MainCamera, *MainScene->GetLights(), *MainScene->GetObjects(), MainShader);
}

void RenderEngine::PostProcessPass()
{
	Post->ExecPPStack(FilterBuffer);
}

Camera * RenderEngine::GetMainCam()
{
	return MainCamera;
}

Shader * RenderEngine::GetMainShader()
{
	return MainShader;
}