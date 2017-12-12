#pragma once

#include "RHI/RenderWindow.h"

#include <vector>
#include <ctime>
#include <time.h>
#include <GLEW\GL\glew.h>
#include <memory>


#include "Core/Transform.h"
#include "Core/GameObject.h"
#include "Core/Input.h"
#include "../Rendering/Core/Material.h"
#include "../Rendering/Renderers/ForwardRenderer.h"
#include "../Rendering/Renderers/DeferredRenderer.h"
#include "UI/TextRenderer.h"
#include "../Rendering/Renderers/RenderSettings.h"
class EditorGizmos;
class UIEditField;
class EditorObjectSelector;
class DebugLineDrawer;
class SceneSerialiser;
class UIManager;
#define USE_THREADING  1
class EditorWindow : public RenderWindow
{


private:

	HDC			m_hdc;				//handle to a device context
	HGLRC		m_hglrc;			//handle to a gl rendering context
	HINSTANCE m_hInstance;
	int			m_width;
	int			m_height;
	static EditorWindow* instance;
	bool DidPhsyx = false;
	//threading
	HANDLE RenderThread;
	HANDLE ThreadStart;
	HANDLE ThreadComplete;

	//timers
	float startms = 0;
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
	bool Once = true;
	bool Allowtimer = true;

	RenderEngine* Renderer;	
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
	Scene* CurrentPlayScene;
	bool IsPlayingScene = false;
	RenderSettings CurrentRenderSettings;
	SceneSerialiser* SceneFileLoader;
	class Editor_Camera* EditorCamera;
	class SceneJSerialiser* Saver;

	float TickRate = 1.0f / 120.0f;
public:
	bool ShowHud = true;
	bool ShowText = true;
	bool LoadText = true;
	bool ExtendedPerformanceStats = true;


	static Scene* GetCurrentScene();
	static UIWidget* CurrentContext;
	static EditorWindow* GetInstance() { return instance; }
	static void SetContext(UIWidget* target);
	static float GetDeltaTime();

	EditorWindow(HINSTANCE hInstance, int width, int height);
	EditorWindow(bool Isdef);
	virtual ~EditorWindow();
	static bool ProcessDebugCommand(std::string command);
	bool CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen);
	int PhysicsThreadLoop();
	static DWORD RunPhysicsThreadLoop(void* pthis);
	BOOL InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height);
	void EnterPlayMode();
	void ExitPlayMode();
	static int GetWidth();
	static int GetHeight();
	void RenderText();
	void Update();
	void SetDeferredState(bool state);

	void PrePhysicsUpdate();

	void DuringPhysicsUpdate();

	

	void		Render();
	void		Resize(int width, int height);
	RenderEngine* GetCurrentRenderer();

	void		DestroyRenderWindow();
	BOOL		MouseLBDown(int x, int y);
	BOOL		MouseLBUp(int x, int y);
	BOOL		MouseRBDown(int x, int y);
	BOOL		MouseRBUp(int x, int y);
	BOOL		MouseMove(int x, int y);
	BOOL		KeyDown(WPARAM key);
	void ProcessMenu(WORD command) override;

};
