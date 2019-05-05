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
struct DeviceDependentObjects
{
	~DeviceDependentObjects();
	Shader_EnvMap* EnvMap = nullptr;
	Shader_Convolution* ConvShader = nullptr;
	Shader_Skybox* SkyboxShader = nullptr;
	void Release();
};
class RenderEngine
{
public:
	RenderEngine(int width, int height);
	virtual ~RenderEngine();
	void Render();
	void PreRender();
	virtual void OnRender() = 0;
	virtual void FinaliseRender() = 0;
	void Init();
	void InitProcessingShaders(DeviceContext * dev);
	void ProcessSceneGPU(DeviceContext * dev);
	void ProcessScene();
	void PrepareData();
	virtual void PostInit() = 0;
	virtual void Resize(int width, int height);
	virtual void DestoryRenderWindow(){};
	//called on play start and in editor when statics are changed
	void StaticUpdate();
	virtual void OnStaticUpdate() = 0;
	virtual void SetScene(Scene* sc);
	virtual void SetEditorCamera(class Editor_Camera* cam);
	class SceneRenderer* SceneRender = nullptr;
	//getters
	Shader* GetMainShader();
	Camera* GetMainCam();
	int GetScaledWidth();
	int GetScaledHeight();
	void HandleCameraResize();
protected:
	void ShadowPass();
	void PostProcessPass();
	int			m_width = 0;
	int			m_height = 0;

	Scene* MainScene = nullptr;
	Camera* MainCamera = nullptr;
	Editor_Camera* EditorCam = nullptr;
	Shader_Main* MainShader = nullptr;
	ShadowRenderer* mShadowRenderer = nullptr;
	PostProcessing* Post = nullptr;
	FrameBuffer* FilterBuffer = nullptr;
	bool once = true;
	DeviceDependentObjects DDOs[MAX_GPU_DEVICE_COUNT];
	int DevicesInUse = 1;
	DynamicResolutionScaler* Scaler = nullptr;
};

