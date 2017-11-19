#pragma once
#include <Windows.h>
#include <string>
#include "../EngineGlobals.h"
#include <ctime>
#include <time.h>

class RenderWindow;
class PhysicsEngine;
class EditorWindow;
class Game;
static long get_nanos(void)
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}
class Engine
{
public:

	EditorWindow* GetEditorWindow();
	static class CompoenentRegistry* CompRegistry;
	static PhysicsEngine*		PhysEngine;
	static class PhysxEngine* PPhysxEngine;
	bool						isWindowVaild = false;
	static std::string GetRootDir();
	Engine();
	~Engine();
	void Destory();
	void SetHInstWindow(HINSTANCE inst) {
		m_hInst = inst;
	}
	RenderWindow* GetWindow() {
		return m_appwnd;
	}
	ERenderSystemType GetCurrentSystem() {
		return CurrentRenderSystem;
	}
	void CreateApplication(HINSTANCE hinst, LPSTR args, int nshow);
	void CreateApplicationWindow(int width, int height, ERenderSystemType type);
	static void setVSync(bool sync);
	bool SwitchRenderAPI(ERenderSystemType type);
	void SetContextData(HGLRC hglrc, HWND hwnd, HDC hdc);
	RenderWindow*				m_appwnd = nullptr;
	static float StartTime;

	static void SetGame(Game* game);
	static Game* GetGame();
	HWND mhwnd;
private:
	
	ERenderSystemType CurrentRenderSystem;
	HINSTANCE m_hInst;
	int mwidth, mheight;
	bool IsDeferedmode = false;
	bool Deferredmode = false;
	bool FullScreen = false;
	HGLRC mhglrc;
	
	HDC mhdc;
	static Game* mgame;

};

