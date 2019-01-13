#include "RenderEngine.h"
#include "RHI/DeviceContext.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "Core/Assets/Scene.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "Editor/Editor_Camera.h"
#include "Rendering/Shaders/Shader_Skybox.h"
#include "Rendering/Shaders/Generation/Shader_EnvMap.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/ParticleSystemManager.h"
#include "Editor/EditorWindow.h"
#include "Editor/EditorCore.h"
RenderEngine::RenderEngine(int width, int height)
{
	m_width = width;
	m_height = height;
	SceneRender = new SceneRenderer(nullptr);
}

RenderEngine::~RenderEngine()
{
	DestoryRenderWindow();
	SafeDelete(SceneRender);
	SafeDelete(mShadowRenderer);
	SafeDelete(Post);
	EnqueueSafeRHIRelease(FilterBuffer);
}

void RenderEngine::Render()
{
	PrepareData();
	if (once)
	{
		RHI::RHIRunFirstFrame();
		once = false;
	}
	if (MainCamera == nullptr)
	{
		return;
	}
	if ((*MainScene->GetMeshObjects()).size() == 0)
	{
		//return;
	}
	PreRender();
	ParticleSystemManager::Get()->Simulate();
	OnRender();
}

void RenderEngine::PreRender()
{
	if (MainScene->StaticSceneNeedsUpdate)
	{
		StaticUpdate();
	}
#if WITH_EDITOR
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
	ParticleSystemManager::Get()->PreRenderUpdate(MainCamera);
}

//init common to both renderers
void RenderEngine::Init()
{
	mShadowRenderer = new ShadowRenderer(SceneRender);
	if (MainScene != nullptr)
	{
		mShadowRenderer->InitShadows(*MainScene->GetLights());
	}

	Conv = ShaderComplier::GetShader<Shader_Convolution>();
	Conv->init();

	envMap = ShaderComplier::GetShader<Shader_EnvMap>();
	envMap->Init();


	PostInit();
	//400mb
	Post = new PostProcessing();
	Post->Init(FilterBuffer);
	SceneRender->Init();
}

void RenderEngine::ProcessScene()
{
	if (MainScene == nullptr)
	{
		return;
	}
	//Todo: TEMP FIX!
	if (RHI::GetFrameCount() > 10)
	{
		return;
	}
	Scene::LightingEnviromentData* Data = MainScene->GetLightingData();
	Conv->TargetCubemap = Data->SkyBox;
	envMap->TargetCubemap = Data->SkyBox;
	RHI::GetDeviceContext(0)->UpdateCopyEngine();
	RHI::GetDeviceContext(0)->CPUWaitForAll();
	RHI::GetDeviceContext(0)->ResetCopyEngine();
	if (Data->DiffuseMap == nullptr)
	{
		Conv->ComputeConvolution(Conv->TargetCubemap);
	}
	envMap->ComputeEnvBRDF();
}

void RenderEngine::PrepareData()
{
	if (MainScene == nullptr)
	{
		return;
	}
	for (size_t i = 0; i < (*MainScene->GetMeshObjects()).size(); i++)
	{
		SceneRender->UpdateUnformBufferEntry(SceneRender->CreateUnformBufferEntry((*MainScene->GetMeshObjects())[i]), (int)i);
	}
	SceneRender->UpdateCBV();
}

void RenderEngine::Resize(int width, int height)
{
	Post->Resize(FilterBuffer);
	Log::OutS << "Resizing to " << GetScaledWidth() << "x" << GetScaledHeight() << Log::OutS;
}

void RenderEngine::StaticUpdate()
{
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->InitShadows(*MainScene->GetLights());
		mShadowRenderer->Renderered = false;
	}
	SceneRender->UpdateLightBuffer(*MainScene->GetLights());
	PrepareData();


	OnStaticUpdate();
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
	SceneRender->SetScene(sc);
	SkyBox->SetSkyBox(sc->GetLightingData()->SkyBox);
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->InitShadows(*MainScene->GetLights());
		mShadowRenderer->Renderered = false;
		ProcessScene();
	}
	if (sc == nullptr)
	{
		MainCamera = nullptr;
	}
	else
	{
		MainCamera = MainScene->GetCurrentRenderCamera();
	}
	HandleCameraResize();
}

void RenderEngine::SetEditorCamera(Editor_Camera * cam)
{
	EditorCam = cam;
}

void RenderEngine::ShadowPass()
{
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->RenderShadowMaps(MainCamera, *MainScene->GetLights(), *MainScene->GetMeshObjects(), MainShader);
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
	return (int)(m_width * RHI::GetRenderSettings()->RenderScale);
}

int RenderEngine::GetScaledHeight()
{
	return (int)(m_height * RHI::GetRenderSettings()->RenderScale);
}

void RenderEngine::HandleCameraResize()
{
#if WITH_EDITOR
	if (MainCamera != nullptr)
	{
		if (EditorWindow::GetEditorCore()->LockAspect)
		{
			MainCamera->UpdateProjection(EditorWindow::GetEditorCore()->LockedAspect);
		}
		else
		{
			MainCamera->UpdateProjection((float)GetScaledWidth() / (float)GetScaledHeight());
		}

	}
#else
	if (MainCamera != nullptr)
	{
		MainCamera->UpdateProjection((float)GetScaledWidth() / (float)GetScaledHeight());
	}
#endif

}

Shader * RenderEngine::GetMainShader()
{
	return MainShader;
}