#pragma once
#include "RHI/RenderWindow.h"
#include "Rendering/Renderers/RenderSettings.h"
class Input;
class Scene;
class GPUPerformanceGraph;
class BaseWindow : public RenderWindow
{
public:
	BaseWindow();
	virtual ~BaseWindow();
	virtual bool CreateRenderWindow(int width, int height) override final;
	virtual void Render() override final;

	virtual void Resize(int width, int height, bool force = false) override final;
	virtual void DestroyRenderWindow() override;
	bool MouseLBDown(int x, int y) override;
	bool MouseLBUp(int x, int y) override;
	bool MouseRBDown(int x, int y) override;
	bool MouseRBUp(int x, int y) override;
	bool MouseMove(int x, int y) override final;
	CORE_API static void StaticResize();


	static bool ProcessDebugCommand(std::string command, std::string & response);
	static class Camera* GetCurrentCamera();
	bool ShowHud = true;
	bool ShowText = false;
	static Scene* GetScene();
	bool IsScenePaused()
	{
		return PauseState;
	}
	void SetPauseState(bool State);

	virtual void OnWindowContextLost() override;
	void EnqueueRestart();
	void SetFrameRateLimit(int limit);
protected:
	void ReLoadCurrentScene();
	virtual Scene* GetCurrentScene();
	//callbacks
	virtual void PostInitWindow(int width, int height)
	{};
	virtual void PreRender()
	{};
	virtual void PostRender()
	{};
	virtual void PostMainPass()
	{};
	virtual void PreDestory()
	{};
	virtual void FixedUpdate();
	virtual void Update()
	{};

	//Sub Modules
	//class RenderEngine * Renderer = nullptr;
	class UIManager* UI = nullptr;
	class DebugLineDrawer* LineDrawer = nullptr;
	class Scene* CurrentScene = nullptr;
	class Editor_Camera* EditorCamera = nullptr;
	class SceneJSerialiser* Saver = nullptr;

	float DeltaTime = 0.0;
	const float TickRate = 1.0f / 60.0f;

	virtual void WindowUI()
	{};
	bool IsRunning = false;
	bool ShouldTickScene = false;
	double TargetDeltaTime = 0.0;
	void LoadScene(std::string RelativePath);
	bool PauseState = false;
	bool StepOnce = false;
	static BaseWindow* Get();
private:
	void PostFrameOne();
	void RenderText();
	void InitilseWindow();
	static BaseWindow* Instance;
	//constants
	bool LoadText = true;
	bool Once = true;
	double AccumTickTime = 0.0f;
	bool RestartNextFrame = false;
	GPUPerformanceGraph* GPUPerfGraph = nullptr;
};

