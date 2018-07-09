#pragma once
#include "RHI/RenderWindow.h"
#include "Core/Input.h"
#include "Rendering/Renderers/ForwardRenderer.h"
#include "Rendering/Renderers/DeferredRenderer.h"
#include "Rendering/Renderers/RenderSettings.h"
class BaseWindow : public RenderWindow
{
public:
	BaseWindow();
	virtual ~BaseWindow();
	virtual bool CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen = false) override final;
	virtual void Render() override final;
	virtual void Resize(int width, int height) override final;
	virtual void DestroyRenderWindow() override;
	BOOL MouseLBDown(int x, int y) override;
	BOOL MouseLBUp(int x, int y) override;
	BOOL MouseRBDown(int x, int y) override;
	BOOL MouseRBUp(int x, int y) override;
	BOOL MouseMove(int x, int y) override final;
	BOOL KeyDown(WPARAM key) override;
	virtual void ProcessMenu(WORD command) override;

	//getters
	static int GetWidth();
	static int GetHeight();
	RenderEngine * GetCurrentRenderer();

	static bool ProcessDebugCommand(std::string command);
	static Camera* GetCurrentCamera();
	bool ShowHud = false;
	bool ShowText = true;
	bool ExtendedPerformanceStats = false;
protected:

	//callbacks
	virtual void PostInitWindow(int width, int height) {};
	virtual void PreRender() {};
	virtual void PostRender() {};
	virtual void PostMainPass() {};
	virtual void PreDestory() {};
	virtual void FixedUpdate();
	virtual void Update() {};
	bool ChangeDisplayMode(int width, int height);
	//Sub Modules
	class RenderEngine * Renderer = nullptr;
	class Input* input = nullptr;
	class UIManager* UI = nullptr;
	class DebugLineDrawer* LineDrawer = nullptr;
	class Scene* CurrentScene = nullptr;
	RenderSettings CurrentRenderSettings;
	class Editor_Camera* EditorCamera = nullptr;
	class SceneJSerialiser* Saver = nullptr;
	
	double DeltaTime = 0.0;
	const float TickRate = 1.0f / 120.0f;

	virtual void WindowUI() {};
	bool IsRunning = false;
	bool ShouldTickScene = false;
	int FrameRateLimit = 0;
	float TargetDeltaTime = 0.0f;
	void LoadScene(std::string RelativePath);

private:
	void PostFrameOne();
	void RenderText();
	void InitilseWindow();
	static BaseWindow* Instance;
	//basics
	HINSTANCE m_hInstance;
	int			m_width;
	int			m_height;
	//constants
	bool IsDeferredMode = false;
	bool LoadText = true;
	bool Once = true;
	float AccumTickTime = 0.0f;	

};

