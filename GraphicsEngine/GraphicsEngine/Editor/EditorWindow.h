#pragma once

#include "RHI/RenderWindow.h"

#include <vector>
//#include <ctime>
//#include <time.h>
//#include <GLEW\GL\glew.h>
//#include <memory>
//
#include "Core/BaseWindow.h"
//#include "Core/Transform.h"
//#include "Core/GameObject.h"
//#include "Core/Input.h"
//#include "../Rendering/Core/Material.h"
//#include "../Rendering/Renderers/ForwardRenderer.h"
//#include "../Rendering/Renderers/DeferredRenderer.h"
//#include "UI/TextRenderer.h"
//#include "../Rendering/Renderers/RenderSettings.h"
class EditorGizmos;
class UIEditField;
class EditorObjectSelector;
class DebugLineDrawer;
class SceneSerialiser;
class UIManager;
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
protected:
	void PrePhysicsUpdate();
	void DuringPhysicsUpdate();
	void FixedUpdate() override;
	void ProcessMenu(WORD command) override;
	void WindowUI() override;
	void Update() override;
	void SetDeferredState(bool state);
private:
	static EditorWindow* instance;
	EditorGizmos* gizmos;
	bool IsDeferredMode = false;
	EditorObjectSelector* selector;
	Scene* CurrentPlayScene;
	bool IsPlayingScene = false;

	class Editor_Camera* EditorCamera;
	class SceneJSerialiser* Saver;


};
