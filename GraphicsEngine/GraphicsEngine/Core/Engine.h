#pragma once
#include "../EngineGlobals.h"
#include <windows.h>
#include <string>

class RenderWindow;
class PhysicsEngine;
class EditorWindow;
class Game;
class Engine
{
public:

	static class CompoenentRegistry* CompRegistry;
	static PhysicsEngine*		PhysEngine;

	bool						isWindowVaild = false;
	static std::string GetRootDir();
	Engine();
	~Engine();
	void Destory();
	void SetHInstWindow(HINSTANCE inst);
	RenderWindow* GetWindow();
	ERenderSystemType GetCurrentSystem();
	void CreateApplication(HINSTANCE hinst, LPSTR args, int nshow);
	static void RunCook();
	void CreateApplicationWindow(int width, int height, ERenderSystemType type);
	static void setVSync(bool sync);
	bool SwitchRenderAPI(ERenderSystemType type);
	RenderWindow*				m_appwnd = nullptr;
	static float StartTime;
	static void SetGame(Game* game);
	static Game* GetGame();
	HWND mhwnd;
private:
	ERenderSystemType ForcedRenderSystem = ERenderSystemType::Limit;
	ERenderSystemType CurrentRenderSystem = ERenderSystemType::Limit;
	HINSTANCE m_hInst;
	int mwidth, mheight;
	bool Deferredmode = false;
	bool FullScreen = false;
	bool ShouldRunCook = false;
	HGLRC mhglrc;	
	HDC mhdc;
	static Game* mgame;
};

