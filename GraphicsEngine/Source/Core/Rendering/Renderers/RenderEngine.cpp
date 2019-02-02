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
	if (RHI::GetMGPUMode()->MainPassSFR)
	{
		DevicesInUse = 2;
	}
	else
	{
		DevicesInUse = 1;
	}
	mShadowRenderer = new ShadowRenderer(SceneRender);
	if (MainScene != nullptr)
	{
		mShadowRenderer->InitShadows(*MainScene->GetLights());
	}
	InitProcessingShaders(RHI::GetDeviceContext(0));
	InitProcessingShaders(RHI::GetDeviceContext(1));
	PostInit();
	//400mb
	Post = new PostProcessing();
	Post->Init(FilterBuffer);
	SceneRender->Init();
}

void RenderEngine::InitProcessingShaders(DeviceContext* dev)
{
	if (dev->GetDeviceIndex() == 0)
	{
		DDOs[dev->GetDeviceIndex()].ConvShader = ShaderComplier::GetShader<Shader_Convolution>(dev);
		DDOs[dev->GetDeviceIndex()].EnvMap = ShaderComplier::GetShader<Shader_EnvMap>(dev);
	}
	else
	{
		DDOs[dev->GetDeviceIndex()].ConvShader = new Shader_Convolution(dev);
		DDOs[dev->GetDeviceIndex()].EnvMap = new Shader_EnvMap(dev);
	}
	DDOs[dev->GetDeviceIndex()].ConvShader->init();
	DDOs[dev->GetDeviceIndex()].EnvMap->Init();
}

void RenderEngine::ProcessSceneGPU(DeviceContext* dev)
{
	Scene::LightingEnviromentData* Data = MainScene->GetLightingData();
	DDOs[dev->GetDeviceIndex()].ConvShader->TargetCubemap = Data->SkyBox;
	DDOs[dev->GetDeviceIndex()].EnvMap->TargetCubemap = Data->SkyBox;
	dev->UpdateCopyEngine();
	dev->CPUWaitForAll();
	dev->ResetCopyEngine();
	if (Data->DiffuseMap == nullptr)
	{
		DDOs[dev->GetDeviceIndex()].ConvShader->ComputeConvolution(DDOs[dev->GetDeviceIndex()].ConvShader->TargetCubemap);
	}
	DDOs[dev->GetDeviceIndex()].EnvMap->ComputeEnvBRDF();

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
	ProcessSceneGPU(RHI::GetDeviceContext(0));
	ProcessSceneGPU(RHI::GetDeviceContext(1));
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
	DDOs[0].SkyboxShader->SetSkyBox(sc->GetLightingData()->SkyBox);
	if (DDOs[1].SkyboxShader != nullptr)
	{
		DDOs[1].SkyboxShader->SetSkyBox(sc->GetLightingData()->SkyBox);
	}
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