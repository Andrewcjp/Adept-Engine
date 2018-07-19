#pragma once
#include "../Core/MinWindows.h"
class WindowsWindow
{
public:
	WindowsWindow();
	virtual ~WindowsWindow();
private:
	static WindowsWindow*		app;
	class Engine*				m_engine;
	BOOL						m_terminate;
	BOOL						MyRegisterClass(HINSTANCE hinst);
	int							mwidth, mheight;
	HWND						HWindow;
	HINSTANCE					m_hInst;
	void						AddMenus(HWND hwnd);
public:	
	static WindowsWindow*		CreateApplication(Engine * EnginePtr, HINSTANCE hInst, LPSTR args, int nshow);
	void						SetVisible(bool visible);
	bool						CreateOSWindow(int width, int height);
	static HWND					GetHWND();
	static void					DestroyApplication();
	static WindowsWindow*		GetApplication();
	int							Run();
	void						Kill();
	static bool					IsActiveWindow();

	static	LRESULT CALLBACK	WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};
typedef WindowsWindow PlatformWindow;
