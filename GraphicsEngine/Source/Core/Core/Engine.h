#pragma once

#include "Core/Platform/Logger.h"
#include "Core/EngineTypes.h"
#include "Physics/PhysicsEngine.h"
#include "Core/EngineSettings.h"
class RenderWindow;
class EditorWindow;
class Game;
class Engine
{
public:
	Engine();
	~Engine();
	CORE_API static class CompoenentRegistry* CompRegistry;
	static PhysicsEngine*			PhysEngine;
	static Engine*					EngineInstance;
	CORE_API static std::string				GetExecutionDir();
	CORE_API static PhysicsEngine*  GetPhysEngineInstance();
	void							PreInit();
	void							OnRender();
	void							OnDestoryWindow();
	void							Destory();
	void							LoadGame();
	RenderWindow*					GetRenderWindow();
	void							CreateApplication();
	static void						RunCook();
	void							CreateApplicationWindow(int width, int height);

	static float					StartTime;
	static void						SetGame(Game* game);
	static Game*					GetGame();
	void							ProcessCommandLineInput(class FString Text, int length);
	int								GetWidth();
	int								GetHeight();
	void							Resize(int width, int height);
	IntPoint*						GetInitalScreen();
	bool							GetWindowValid() const;
	CORE_API static void			Exit(int code = 0);
	void							HandleInput(unsigned int key);
	static bool						GetIsCooking();
	static Engine*					Get();
	static const EngineSettings*	GetSettings() { return &EngineInstance->Settings; }
	static float					GetPhysicsDeltaTime();
private:
	ERenderSystemType				ForcedRenderSystem = ERenderSystemType::Limit;
	int								mwidth, mheight;
	IntPoint						IntialScreenSize;
	bool							Deferredmode = false;
	bool							FullScreen = false;
	bool							ShouldRunCook = false;
	static Game*					mgame;
	bool							isWindowVaild = false;
	RenderWindow*					m_appwnd = nullptr;
	bool							IsCooking = false;
	EngineSettings					Settings;
};

