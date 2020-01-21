#pragma once
#include "RHI/RenderWindow.h"
#include "Core/BaseWindow.h"
#include "Core/EngineTypes.h"
class EditorGizmos;
class UIEditField;
class EditorObjectSelector;
class DebugLineDrawer;
class SceneSerialiser;
class UIManager;
class EditorCore;
class Scene;
#define PLAYMODE_USE_SAVED 1
#if WITH_EDITOR
class EditorWindow : public BaseWindow
{
public:
	void PostInitWindow(int w, int h) override;
	CORE_API static EditorWindow* GetInstance();
	EditorWindow();
	virtual ~EditorWindow();
	void EnterPlayMode();
	void ExitPlayMode();
	virtual void DestroyRenderWindow() override final;
	bool MouseLBDown(int x, int y) override;
	static EditorCore* GetEditorCore();
	bool UseSmallerViewPort();
	IntRect GetViewPortRect();
	void Eject();
	CORE_API bool IsEditorEjected();
	bool IsInPlayMode();;
	class Editor_Camera* EditorCamera = nullptr;
protected:
	bool IsSceneRunning();
	Scene* GetCurrentScene() override;
	void PrePhysicsUpdate();
	void DuringPhysicsUpdate();
	void FixedUpdate() override;

	void LoadScene();
	void RefreshScene();
	void WindowUI() override;
	void Update() override;
	void SaveScene();
private:
	bool StartSimulate = false;
	static EditorWindow* instance;
	EditorGizmos* gizmos = nullptr;
	EditorObjectSelector* selector = nullptr;
	Scene* CurrentPlayScene = nullptr;
	bool IsPlayingScene = false;
	std::string CurrentSceneSavePath;
	std::string EditorPlaySceneTempFile = "";
	EditorCore* mEditorCore = nullptr;

};
#endif