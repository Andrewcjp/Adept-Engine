#pragma once
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
};

