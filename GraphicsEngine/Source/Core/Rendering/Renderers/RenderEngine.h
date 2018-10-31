#pragma once
#include "Rendering/Core/Camera.h"
#include "Core/GameObject.h"
#include "Rendering/Core/Light.h"
#include "Rendering/Shaders/Shader_Main.h"

#include "Rendering/Core/FrameBuffer.h"
#include "RenderSettings.h"

class Scene;
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
	void ProcessScene();
	void PrepareData();
	virtual void PostInit() =0;
	virtual void Resize(int width, int height);
	virtual void DestoryRenderWindow() {};
	//called on play start and in editor when statics are changed
	void StaticUpdate();
	virtual void OnStaticUpdate() =0;
	virtual void SetScene(Scene* sc);
	virtual void SetEditorCamera(class Editor_Camera* cam);
	class SceneRenderer* SceneRender = nullptr;
	//getters
	Shader* GetMainShader();
	Camera* GetMainCam();
	int GetScaledWidth();
	int GetScaledHeight();
protected:
	class Shader_Convolution * Conv = nullptr;
	class Shader_EnvMap* envMap = nullptr;
	void ShadowPass();
	void PostProcessPass();
	int			m_width = 0;
	int			m_height = 0;
	
	Scene* MainScene = nullptr; 
	Camera* MainCamera = nullptr;
	Editor_Camera* EditorCam = nullptr;
	Shader_Main* MainShader = nullptr;
	class ShadowRenderer* mShadowRenderer = nullptr;
	class PostProcessing* Post = nullptr;
	FrameBuffer* FilterBuffer = nullptr;
	class Shader_Skybox* SkyBox = nullptr;
	bool once = true;
};

