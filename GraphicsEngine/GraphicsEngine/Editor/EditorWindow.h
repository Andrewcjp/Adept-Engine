#pragma once

#include "RHI/RenderWindow.h"

#include <vector>

#include "Core/Transform.h"
#include "Core/GameObject.h"
#include "../Rendering/Core/Light.h"

#include <ctime>
#include <time.h>
#include <GLEW\GL\glew.h>

#include "Core/Input.h"
#include <memory>
#include "Core/Input.h"
#include "UI/TextRenderer.h"

#include "OpenGL\OGLTexture.h"
#include "../Rendering/Core/Material.h"
#include "../Rendering/Renderers/ForwardRenderer.h"
#include "../Rendering/Renderers/DeferredRenderer.h"
#include "UI\UIManager.h"
#include "../Rendering/Renderers/RenderSettings.h"
class EditorGizmos;
class UIEditField;
class EditorObjectSelector;
class DebugLineDrawer;
class EditorWindow : public RenderWindow
{


private:

	int itemsrender = 0;
	float	    m_euler[3];
	HDC			m_hdc;				//handle to a device context
	HGLRC		m_hglrc;			//handle to a gl rendering context
	HINSTANCE m_hInstance;
	int			m_width;
	int			m_height;
	static EditorWindow* instance;
	clock_t tstart;
	const int MaxPhysicsObjects = 1000;

	long lasttime = 0;
	float deltatime = 1;
	double accumilatePhysxdeltatime = 0;
	double accumrendertime = 0;
	double currenfps = 0;
	double avgtime = 0;
	int framecount = 0;
	int avgaccum = 0;
	double fpsnexttime = 0;
	float timesincestat = 0;
	double fpsaccumtime = 0.25f;
	double startms = 0;
	double physxtime = 0;
	double ShadowTime = 0;
	double FinalTime = 0;
	GLint64 GPUStartTime = 0;
	float GPUtime = 0;
	bool Allowtimer = true;
	GLuint query = 0;
	double RenderTime = 0;
	double sleeptimeMS = 0;
	double ShadowRendertime = 0;

	float CPUTime = 0;
	RenderEngine* Renderer;
	bool Once = true;
	EditorGizmos* gizmos;
	std::vector<GameObject*> PhysicsObjects;

	bool RenderedReflection = false;
	int FrameBufferRatio = 1;
	bool IsDeferredMode = false;
	std::unique_ptr<Input> input;
	std::unique_ptr<UIManager> UI;
	EditorObjectSelector* selector;
	DebugLineDrawer* dLineDrawer;
	Scene* CurrentScene;
	RenderSettings CurrentRenderSettings;
protected:

public:
	static UIWidget* CurrentContext;
	static void SetContext(UIWidget* target);
	static float GetDeltaTime()
	{
		if (instance != nullptr)
		{
			return instance->deltatime;
		}
		return 0;
	}
	EditorWindow(HINSTANCE hInstance, int width, int height);
	EditorWindow(bool Isdef);
	virtual ~EditorWindow();
	static float GetFrameTime();
	static float GetCPUTime();
	static bool ProcessDebugCommand(std::string command);
	bool IsFullscreen = false;
	void SwitchFullScreen(HINSTANCE hInstance);
	bool CreateRenderWindow(HINSTANCE hInstance, int width, int height);
	BOOL InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height);
	static int GetWidth()
	{
		if (instance != nullptr)
		{
			return instance->m_width;
		}
		return 0;
	}
	static int GetHeight()
	{
		if (instance != nullptr)
		{
			return instance->m_height;
		}
		return 0;
	}
	void RenderText();
	void Update();
	void SetDeferredState(bool state)
	{
		IsDeferredMode = state;
	}
	bool ShowHud = true;
	bool ShowText = true;
	bool LoadText = true;
	bool ExtendedPerformanceStats = true;
	void AddPhysObj(GameObject* go)
	{
		Renderer->AddPhysObj(go);
	}

	void		Render();
	void		Resize(int width, int height);
	RenderEngine* GetCurrentRenderer()
	{
		return Renderer;
	}

	void		DestroyRenderWindow();

	BOOL		MouseLBDown(int x, int y);
	BOOL		MouseLBUp(int x, int y);
	BOOL		MouseRBDown(int x, int y);
	BOOL		MouseRBUp(int x, int y);
	BOOL		MouseMove(int x, int y);
	BOOL		KeyDown(WPARAM key);
	void ProcessMenu(WORD command) override
	{
		switch (command)
		{
		case 4://add gameobject

			break;
		default:
			break;
		}
	}
};
