#pragma once
#ifdef PLATFORM_WINDOWS
#include "Core/MinWindows.h"
#include "Core/EngineTypes_fwd.h"
#include "Core/Platform/Generic/GenericWindow.h"

class WindowsWindow : public GenericWindow
{
public:
	WindowsWindow();
	virtual						~WindowsWindow();
	void						GetDesktopResolution(int& horizontal, int& vertical);
	IntPoint					GetMousePos();
	void						SetMousePos(IntPoint& point);
	char						GetVirtualKeyAsChar(unsigned int key);
	short						GetCharAsVirtualKey(char c);
	bool						IsKeyDown(short key);
	static void					SetCursorType(GenericWindow::CursorType Type);
	static WindowsWindow*		CreateApplication(Engine * EnginePtr, HINSTANCE hInst, LPSTR args, int nshow);
	void						SetVisible(bool visible);
	static void					Maximize();
	bool						CreateOSWindow(int width, int height);
	CORE_API static HWND		GetHWND();
	static void					DestroyApplication();
	static WindowsWindow*		GetApplication();
	int							Run();
	
	static void					Kill(int code);
	static bool					IsActiveWindow();
	void						SetupHPMI();
	CORE_API static HINSTANCE   GetHInstance();

	static	LRESULT CALLBACK	WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	static void					DestorySplashWindow();

	static void					TickSplashWindow(int amt = 1, std::string Section = std::string());
private:
	static void					EmptyMessageQueue();
	static WindowsWindow*		app;
	class Engine*				m_engine;
	BOOL						m_terminate;
	BOOL						MyRegisterClass(HINSTANCE hinst);
	int							mwidth, mheight;
	HWND						HWindow;
	HINSTANCE					m_hInst;
	HKL							Layout;

	HCURSOR						Cursor;
	HCURSOR						Cursors[GenericWindow::CursorType::Limit];
	void						CreateSplashWindow();
	HWND						SplashWindow;
	HWND						ProgressBar;
	HWND						Label;
	int							CurrentProgress = 0;
};

#endif