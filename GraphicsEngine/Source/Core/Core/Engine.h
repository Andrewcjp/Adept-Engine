#pragma once

#include "Core/Platform/Logger.h"
#include "Core/EngineTypes.h"
#include "Physics/PhysicsEngine.h"
#include "Core/EngineSettings.h"
class RenderWindow;
class EditorWindow;
class Game;
struct EnginePersistentData;
namespace Threading
{
	class TaskGraph;
}
class Engine
{
public:
	static const int RestartCode = -101;
	Engine(EnginePersistentData* epd);
	~Engine();
	CORE_API static class CompoenentRegistry* CompRegistry;
	static PhysicsEngine*			PhysEngine;
	static Engine*					EngineInstance;
	CORE_API static std::string		GetExecutionDir();
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
	void							TestTDPhysics();
	static float					StartTime;
	static void						SetGame(Game* game);
	static Game*					GetGame();
	void							ProcessCommandLineInput(class FString Text, int length);
	int								GetWidth();
	int								GetHeight();
	void							Resize(int width, int height);
	IntPoint*						GetInitalScreen();
	bool							GetWindowValid() const;
	CORE_API static void			RequestExit(int code);

	void							HandleInput(unsigned int key);
	void							HandleKeyUp(unsigned int key);
	static bool						GetIsCooking();
	static Engine*					Get();
	static const EngineSettings*	GetSettings()
	{
		return &EngineInstance->Settings;
	}
	CORE_API static float			GetPhysicsDeltaTime();
	CORE_API static float			GetDeltaTime();
	static Threading::TaskGraph*	GetTaskGraph();
	static bool IsSecondLoad();
	CORE_API static EnginePersistentData* GetEPD();
private:
	EnginePersistentData*			EPD = nullptr;
	static void						Exit(int code = 0);
	ERenderSystemType				ForcedRenderSystem = ERenderSystemType::Limit;
	int								mwidth, mheight;
	IntPoint						IntialScreenSize;
	bool							ShouldRunCook = false;
	static Game*					mgame;
	bool							isWindowVaild = false;
	RenderWindow*					m_appwnd = nullptr;
	bool							IsCooking = false;
	EngineSettings					Settings;
	Threading::TaskGraph*			TaskGraph = nullptr;

};

