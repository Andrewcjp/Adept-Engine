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
#include "../Core/DynamicResolutionScaler.h"
#include "../Core/Culling/CullingManager.h"
#include "RHI/RHI.h"
#include "../VR/HMD.h"
#include "../VR/HMDManager.h"
#include "../Core/RenderingUtils.h"
#include "../Shaders/PostProcess/Shader_Compost.h"
#include "../Core/Material.h"
#include "../Core/RelfectionProbe.h"
#include "Core/Performance/PerfManager.h"
#include "../Core/DebugLineDrawer.h"

RenderEngine::RenderEngine(int width, int height)
{
	m_width = width;
	m_height = height;
	SceneRender = new SceneRenderer(nullptr);
	Scaler = new DynamicResolutionScaler();
	Culling = new CullingManager();
	ScreenWriteList = RHI::CreateCommandList(ECommandListType::Graphics);
}

RenderEngine::~RenderEngine()
{
	DestoryRenderWindow();
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (i != 0)
		{
			DDOs[i].Release();
		}
	}
	SafeDelete(SceneRender);
	SafeDelete(mShadowRenderer);
	SafeDelete(Post);
	SafeDelete(Culling);
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
	SceneRender->UpdateLightBuffer(*MainScene->GetLights());
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
	Scaler->Tick();
	//if using VR FS culling in run just before each eye
	if (!RHI::IsRenderingVR())
	{
		Culling->UpdateMainPassFrustumCulling(MainCamera, MainScene);
	}
}

//init common to both renderers
void RenderEngine::Init()
{
	if (RHI::GetMGPUSettings()->MainPassSFR)
	{
		DevicesInUse = 2;
	}
	else
	{
		DevicesInUse = 1;
	}
	mShadowRenderer = new ShadowRenderer(SceneRender, Culling);
	if (MainScene != nullptr)
	{
		mShadowRenderer->InitShadows(*MainScene->GetLights());
	}
	InitProcessingShaders(RHI::GetDeviceContext(0));
	InitProcessingShaders(RHI::GetDeviceContext(1));
	CubemapCaptureList = RHI::CreateCommandList(ECommandListType::Graphics);
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
	desc.ShaderInUse = Material::GetDefaultMaterialShader();
	desc.FrameBufferTarget = DDOs[0].MainFrameBuffer;
	CubemapCaptureList->SetPipelineStateDesc(desc);
	PostInit();
	Post = new PostProcessing();
	Post->Init(DDOs[0].MainFrameBuffer);
	SceneRender->Init();
	SceneRender->SB = DDOs[0].SkyboxShader;

	//debug 
	SceneRender->probes.push_back(new RelfectionProbe());
	//SceneRender->probes[0]->ProbeMode = EReflectionProbeMode::ERealTime;
}

void RenderEngine::InitProcessingShaders(DeviceContext* dev)
{
	if (dev == nullptr)
	{
		return;
	}
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
	if (dev == nullptr)
	{
		return;
	}
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
	//#Scene TEMP FIX!
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
		(*MainScene->GetMeshObjects())[i]->PrepareDataForRender();
	}
}

void RenderEngine::Resize(int width, int height)
{
	Post->Resize(DDOs[0].MainFrameBuffer);
	if (MainCamera != nullptr)
	{
		MainCamera->UpdateProjection((float)width / (float)height);
	}
	if (RHI::IsRenderingVR())
	{
		RHI::GetHMD()->UpdateProjection((float)width / (float)height);
	}
	int ApoxPValue = glm::iround((float)GetScaledWidth() / (16.0f / 9.0f));
	Log::OutS << "Resizing to " << GetScaledWidth() << "x" << GetScaledHeight() << " approx: " << ApoxPValue << "P " << Log::OutS;
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
#if !BASIC_RENDER_ONLY
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->InitShadows(*MainScene->GetLights());
		mShadowRenderer->Renderered = false;
		ProcessScene();
	}
#endif
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
#if !BASIC_RENDER_ONLY
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->RenderShadowMaps(MainCamera, *MainScene->GetLights(), *MainScene->GetMeshObjects(), MainShader);
	}
#endif
}

void RenderEngine::PostProcessPass()
{
	Post->ExecPPStack(&DDOs[0]);
}

void RenderEngine::PresentToScreen()
{
	ScreenWriteList->ResetList();
	ScreenWriteList->SetScreenBackBufferAsRT();
	ScreenWriteList->ClearScreen();
	if (RHI::IsRenderingVR())
	{
		if (RHI::GetVrSettings()->MirrorMode == EVRMirrorMode::Both)
		{
			RHIPipeLineStateDesc D = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_VROutput>());
			D.Cull = false;
			ScreenWriteList->SetPipelineStateDesc(D);

			ScreenWriteList->SetFrameBufferTexture(DDOs[0].MainFrameBuffer, EEye::Left);
			ScreenWriteList->SetFrameBufferTexture(DDOs[0].RightEyeFramebuffer, EEye::Right);
		}
		else
		{
			RHIPipeLineStateDesc D = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_Compost>());
			D.Cull = false;
			ScreenWriteList->SetPipelineStateDesc(D);
			if (RHI::GetVrSettings()->MirrorMode == EVRMirrorMode::Left)
			{
				ScreenWriteList->SetFrameBufferTexture(DDOs[0].MainFrameBuffer, 0);
			}
			else if (RHI::GetVrSettings()->MirrorMode == EVRMirrorMode::Right)
			{
				ScreenWriteList->SetFrameBufferTexture(DDOs[0].RightEyeFramebuffer, 0);
			}
		}
	}
	else
	{
		RHIPipeLineStateDesc D = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_Compost>());
		D.Cull = false;
		ScreenWriteList->SetPipelineStateDesc(D);
		ScreenWriteList->SetFrameBufferTexture(DDOs[0].MainFrameBuffer, 0);
	}
	RenderingUtils::RenderScreenQuad(ScreenWriteList);
	ScreenWriteList->Execute();
	if (RHI::IsRenderingVR())
	{
		RHI::GetHMD()->OutputToEye(DDOs[0].MainFrameBuffer, EEye::Right);
		RHI::GetHMD()->OutputToEye(DDOs[0].RightEyeFramebuffer, EEye::Left);
	}
}

void RenderEngine::UpdateMVForMainPass()
{
	if (RHI::IsRenderingVR())
	{
		VRCamera* VRCam = RHI::GetHMD()->GetVRCamera();
		SceneRender->UpdateMV(VRCam);
	}
	else
	{
		SceneRender->UpdateMV(MainCamera);
	}
}



Camera * RenderEngine::GetMainCam()
{
	return MainCamera;
}

int RenderEngine::GetScaledWidth()
{
	if (RHI::GetRenderSettings()->LockBackBuffer)
	{
		return RHI::GetRenderSettings()->LockedWidth;
	}
	else
	{
		return (int)(m_width * RHI::GetRenderSettings()->GetCurrentRenderScale());
	}
}

int RenderEngine::GetScaledHeight()
{
	if (RHI::GetRenderSettings()->LockBackBuffer)
	{
		return RHI::GetRenderSettings()->LockedHeight;
	}
	else
	{
		return (int)(m_height * RHI::GetRenderSettings()->GetCurrentRenderScale());
	}
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

DeviceDependentObjects::~DeviceDependentObjects()
{
	SafeDelete(SkyboxShader);
}

void DeviceDependentObjects::Release()
{

}

void RenderEngine::CubeMapPass()
{
	if (!SceneRender->AnyProbesNeedUpdate())
	{
		return;
	}
	CubemapCaptureList->ResetList();
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader());
	CubemapCaptureList->SetPipelineStateDesc(Desc);
	if (mShadowRenderer != nullptr)
	{
		mShadowRenderer->BindShadowMapsToTextures(CubemapCaptureList, true);
	}
	CubemapCaptureList->SetFrameBufferTexture(DDOs[CubemapCaptureList->GetDeviceIndex()].ConvShader->CubeBuffer, MainShaderRSBinds::DiffuseIr);
	if (MainScene->GetLightingData()->SkyBox != nullptr)
	{
		CubemapCaptureList->SetTexture(MainScene->GetLightingData()->SkyBox, MainShaderRSBinds::SpecBlurMap);
	}
	CubemapCaptureList->SetFrameBufferTexture(DDOs[CubemapCaptureList->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, MainShaderRSBinds::EnvBRDF);
	SceneRender->UpdateRelflectionProbes(CubemapCaptureList);

	CubemapCaptureList->Execute();
}


void RenderEngine::RenderDebug(FrameBuffer* FB, RHICommandList* list, EEye::Type eye)
{
	PerfManager::StartTimer("LineDrawer");
	DebugLineDrawer::Get()->RenderLines(FB, list, eye);
	DebugLineDrawer::Get2()->RenderLines(FB, list, eye);
	PerfManager::EndTimer("LineDrawer");
}

void RenderEngine::UpdateMainPassCulling(EEye::Type eye)
{

}