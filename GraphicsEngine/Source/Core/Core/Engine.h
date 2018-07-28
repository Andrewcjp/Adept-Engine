#pragma once
#include "EngineGlobals.h"

#include <string>
#include "Core/Platform/Logger.h"
#include "Core/EngineTypes.h"
class RenderWindow;
class PhysicsEngine;
class EditorWindow;
class Game;
class Engine
{
public:

	static PhysicsEngine*		PhysEngine;
	CORE_API static class CompoenentRegistry* CompRegistry;
	CORE_API static PhysicsEngine* GetPhysEngineInstance();
	bool						isWindowVaild = false;
	static Engine* EngineInstance;
	static std::string GetRootDir();
	Engine();
	~Engine();
	void PreInit();
	void OnRender();
	void OnDestoryWindow();
	void Destory();
	void LoadGame();
	RenderWindow* GetRenderWindow();
	void CreateApplication();
	static void RunCook();
	void CreateApplicationWindow(int width, int height);
	RenderWindow*				m_appwnd = nullptr;
	static float StartTime;
	static void SetGame(Game* game);
	static Game* GetGame();
	void ProcessCommandLineInput(class FString Text,int length);
	int GetWidth();
	int GetHeight();
	void Resize(int width, int height);
	IntPoint* GetInitalScreen();
private:
	ERenderSystemType ForcedRenderSystem = ERenderSystemType::Limit;
	int mwidth, mheight;
	IntPoint IntialScreenSize;
	bool Deferredmode = false;
	bool FullScreen = false;
	bool ShouldRunCook = false;
	static Game* mgame;
};

