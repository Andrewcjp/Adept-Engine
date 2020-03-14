#pragma once
#include "MinWindows.h"

struct IntPoint;

class GenericWindow
{
public:
	enum CursorType
	{
		Normal,
		Drag,
		IBeam,
		Limit
	};
	void Destory()
	{};
	static void SetCursorType(GenericWindow::CursorType Type);
	static void					DestorySplashWindow();
	static void					TickSplashWindow(int amt = 1, std::string Section = std::string());
	static void					DestroyApplication();
	static GenericWindow*		GetApplication();

	static void					Kill(int code);
	static bool					IsActiveWindow();
	void						GetDesktopResolution(int& horizontal, int& vertical);
	IntPoint					GetMousePos();
	void						SetMousePos(IntPoint& point);
	char						GetVirtualKeyAsChar(unsigned int key);
	short						GetCharAsVirtualKey(char c);
	bool						IsKeyDown(short key);
	CORE_API static HWND		GetHWND() { return HWND(); };
};

