#pragma once
#include "Rendering/Core/Camera.h"
#include "Core/GameObject.h"
#include "Rendering/Core/Light.h"
#include "Rendering/Shaders/Shader_Main.h"

#include "Rendering/Core/FrameBuffer.h"
#include "RenderSettings.h"
#define ENABLE_RENDERER_DEBUGGING WITH_EDITOR

class Scene;
class ShadowRenderer;
class PostProcessing;
class Shader_Skybox;
class Shader_Convolution;
class Shader_EnvMap;
class DynamicResolutionScaler;
class CullingManager;
class Shader_Deferred;
class LightCullingEngine;
struct DeviceDependentObjects
{
	~DeviceDependentObjects();
	Shader_EnvMap* EnvMap = nullptr;
	Shader_Convolution* ConvShader = nullptr;
	Shader_Skybox* SkyboxShader = nullptr;
	RHICommandList* MainCommandList[EEye::Limit] = { nullptr };

	FrameBuffer* MainFrameBuffer = nullptr;
#if SUPPORTVR
	FrameBuffer* RightEyeFramebuffer = nullptr;
#endif

	FrameBuffer* Gbuffer = nullptr;
	RHICommandList* GbufferWriteList[EEye::Limit] = { nullptr };
	Shader_Deferred* DeferredShader = nullptr;
#if SUPPORTVR
	FrameBuffer* RightEyeGBuffer = nullptr;
#endif
	RHICommandList* DebugCommandList = nullptr;
	void Release();
	FrameBuffer* GetGBuffer(EEye::Type e);
	FrameBuffer* GetMain(int e);
	FrameBuffer* RTBuffer = nullptr;

};
class RenderEngine
{
public:
	RenderEngine(int width, int height);
	virtual ~RenderEngine();
	void Render();
	void PreRender();
	virtual void OnRender() = 0;
	void Init();
	void InitProcessingShaders(DeviceContext * dev);
	void ProcessSceneGPU(DeviceContext * dev);
	void ProcessScene();
	void PrepareData();
	virtual void PostInit() = 0;
	virtual void Resize(int width, int height);
	virtual void DestoryRenderWindow()
	{};
	//called on play start and in editor when statics are changed
	void StaticUpdate();
	virtual void OnStaticUpdate() = 0;
	virtual void SetScene(Scene* sc);
	virtual void SetEditorCamera(class Editor_Camera* cam);
	virtual FrameBuffer* GetGBuffer() = 0;
	class SceneRenderer* SceneRender = nullptr;
	//getters
	Shader* GetMainShader();

	Camera* GetMainCam();
	int GetScaledWidth();
	int GetScaledHeight();
	void HandleCameraResize();
	virtual void PostProcessPass();
	void PresentToScreen();
	void UpdateMVForMainPass();
	ShadowRenderer* mShadowRenderer = nullptr;
	void RunLightCulling();
	LightCullingEngine* LightCulling = nullptr;
	DynamicResolutionScaler* GetDS();
	
protected:
	void RenderDebugPass();

	void RunReflections(DeviceDependentObjects * d, EEye::Type eye = EEye::Left);

	void ShadowPass();
	void CubeMapPass();
	void RenderDebug(FrameBuffer * FB, RHICommandList * list, EEye::Type eye);
	void UpdateMainPassCulling(EEye::Type eye);
	virtual void PostSizeUpdate();
	int			m_width = 0;
	int			m_height = 0;

	Scene* MainScene = nullptr;
	Camera* MainCamera = nullptr;
	Editor_Camera* EditorCam = nullptr;
	Shader_Main* MainShader = nullptr;
	
	PostProcessing* Post = nullptr;
	bool once = true;
	DeviceDependentObjects DDOs[MAX_GPU_DEVICE_COUNT];
	int DevicesInUse = 1;
	DynamicResolutionScaler* Scaler = nullptr;
	CullingManager* Culling = nullptr;
	//used to write the final image to the back buffer
	RHICommandList* ScreenWriteList = nullptr;
	RHICommandList* CubemapCaptureList = nullptr;
	
};

