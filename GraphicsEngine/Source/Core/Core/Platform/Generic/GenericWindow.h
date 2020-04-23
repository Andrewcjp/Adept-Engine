#pragma once
#include "MinWindows.h"

struct IntPoint;
struct PlatformContextMenu
{
	std::vector<std::string> MenuItems;
	glm::ivec2 MenuPos = glm::ivec2();
	void AddItem(std::string name)
	{
		MenuItems.push_back(name);
	}
};
struct PlatformMenuBar
{
	std::string MenuName;
	std::vector<std::string> MenuItems;
	std::function<void(int CMDIndex)> SingleHandleFuncPTR;
	void AddItem(std::string name)
	{
		MenuItems.push_back(name);
	}
	//Internal data
	int MenuCMDOffset;
	void Execute(int index)
	{
		if (SingleHandleFuncPTR != nullptr)
		{
			SingleHandleFuncPTR(index);
		}
	}
};
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

