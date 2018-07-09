#include "RenderEngine.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "Editor/Editor_Camera.h"
#include "Rendering/Shaders/Shader_Skybox.h"
#include "Rendering/Shaders/Generation/Shader_Convolution.h"
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
#endif
	{
		MainCamera = MainScene->GetCurrentRenderCamera();
	}
}
#include "../RHI/DeviceContext.h"
//init common to both renderers
void RenderEngine::Init()
{
	mShadowRenderer = new ShadowRenderer();
	if (MainScene != nullptr)
	{
		mShadowRenderer->InitShadows(*MainScene->GetLights());
	}

	Conv = new Shader_Convolution();
	Conv->init();
	Conv->TargetCubemap = AssetManager::DirectLoadTextureAsset("\\asset\\texture\\cube_1024_preblurred_angle3_ArstaBridge.dds", true);
	RHI::GetDeviceContext(0)->UpdateCopyEngine();
	RHI::GetDeviceContext(0)->CPUWaitForAll();
	RHI::GetDeviceContext(0)->ResetCopyEngine();
	Conv->ComputeConvolution(Conv->TargetCubemap);

	GPUStateCache::Create();
	PostInit();
	Post = new PostProcessing();
	Post->Init(FilterBuffer);
}

void RenderEngine::PrepareData()
{
	for (size_t i = 0; i < (*MainScene->GetObjects()).size(); i++)
	{
		MainShader->UpdateUnformBufferEntry(MainShader->CreateUnformBufferEntry((*MainScene->GetObjects())[i]), (int)i);
	}
}

void RenderEngine::Resize(int width, int height)
{
	if (RHI::IsD3D12())
	{
		if (D3D12RHI::Instance)
		{
			D3D12RHI::Instance->ResizeSwapChain(width, height, true);
		}
	}
	Post->Resize(FilterBuffer);
	Log::OutS  << "Resizing to " << GetScaledWidth() << "x" << GetScaledHeight() << Log::OutS;
}



void RenderEngine::StaticUpdate()
{
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->InitShadows(*MainScene->GetLights());
		mShadowRenderer->Renderered = false;
	}
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
//	Conv->ComputeConvolution(Conv->TargetCubemap);
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->RenderShadowMaps(MainCamera, *MainScene->GetLights(), *MainScene->GetObjects(), MainShader);
	}
}

void RenderEngine::PostProcessPass()
{
	Post->ExecPPStack(FilterBuffer);
}

Camera * RenderEngine::GetMainCam()
{
	return MainCamera;
}

int RenderEngine::GetScaledWidth()
{
	return m_width * CurrentRenderSettings.RenderScale;
}

int RenderEngine::GetScaledHeight()
{
	return m_height * CurrentRenderSettings.RenderScale;
}

Shader * RenderEngine::GetMainShader()
{
	return MainShader;
}