#pragma once

#include "RHI/RenderWindow.h"

#include <vector>
#include "Core/BaseWindow.h"
class EditorGizmos;
class UIEditField;
class EditorObjectSelector;
class DebugLineDrawer;
class SceneSerialiser;
class UIManager;
#define PLAYMODE_USE_SAVED 1
class EditorWindow : public BaseWindow
{

public:
	bool ShowHud = true;
	bool ShowText = true;
	bool LoadText = true;
	bool ExtendedPerformanceStats = true;

	void PostInitWindow(int w, int h) override;
	static Scene* GetCurrentScene();
	static EditorWindow* GetInstance() { return instance; }
	EditorWindow(HINSTANCE hInstance, int width, int height);
	EditorWindow(bool Isdef);
	virtual ~EditorWindow();
	void EnterPlayMode();
	void ExitPlayMode();

	BOOL MouseLBDown(int x, int y) override;
protected:
	void PrePhysicsUpdate();
	void DuringPhysicsUpdate();
	void FixedUpdate() override;
	void LoadScene();
	void RefreshScene();
	void ProcessMenu(WORD command) override;
	void WindowUI() override;
	void Update() override;
	void SaveScene();
	void SetDeferredState(bool state);
private:
	static EditorWindow* instance;
	EditorGizmos* gizmos;
	bool IsDeferredMode = false;
	EditorObjectSelector* selector;
	Scene* CurrentPlayScene;
	bool IsPlayingScene = false;
	int statcount = 0;
	int currentmemeory = 0;
	class Editor_Camera* EditorCamera;
	class SceneJSerialiser* Saver;
	std::string CurrentSceneSavePath;


};
