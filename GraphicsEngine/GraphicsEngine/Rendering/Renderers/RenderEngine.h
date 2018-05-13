#pragma once
#include "../Rendering/Core/Camera.h"
#include "Core/GameObject.h"
#include "../Rendering/Core/Light.h"
#include "Rendering\Shaders\Shader_Main.h"
#include <vector>
#include "../Rendering/Core/FrameBuffer.h"
#include "Rendering\Shaders\ShaderOutput.h"
#include "RenderSettings.h"
class OGLWindow;
class Scene;
class RenderEngine
{
public:
	RenderEngine(int width, int height) {
		m_width = width;
		m_height = height;
	}
	~RenderEngine();
	virtual void Render() = 0;
	virtual void FinaliseRender() = 0;
	virtual void Init() = 0;
	virtual Camera* GetMainCam() = 0;
	virtual void AddGo(GameObject* g) = 0;
	virtual void AddLight(Light* l) = 0;
	virtual void Resize(int width, int height) = 0;
	virtual Shader* GetMainShader() = 0;
	virtual std::vector<GameObject*> GetObjects() = 0;
	virtual FrameBuffer* GetReflectionBuffer() = 0;
	virtual ShaderOutput* GetFilterShader() = 0;
	bool OcclusionQuerry = true;
	virtual void DestoryRenderWindow() = 0;
	virtual void SetRenderSettings(RenderSettings settings);
	virtual void SetScene(Scene* sc);
	virtual void SetEditorCamera(class Editor_Camera* cam);
protected:
	int			m_width;
	int			m_height;
	RenderSettings settings;
	Scene* mainscene = nullptr;    
	Editor_Camera* EditorCam = nullptr;
};

