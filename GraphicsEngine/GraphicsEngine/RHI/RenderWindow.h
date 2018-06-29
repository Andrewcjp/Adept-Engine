#pragma once



class RenderEngine;
class GameObject;
class RenderWindow
{
protected:
	HWND		m_hwnd;				//handle to a window
	int			m_width;
	int			m_height;

public:
	RenderWindow() { ; }
	virtual				~RenderWindow() { ; }

	//	virtual BOOL		InitWindow(HINSTANCE hInstance, int width, int height) = 0;
	virtual bool CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen = false) =0;
	/*virtual BOOL InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height)=0;*/
	virtual void		Render() = 0;
	virtual void		Resize(int width, int height) = 0;

	void				SetVisible(BOOL visible)
	{
		ShowWindow(m_hwnd, visible ? SW_SHOW : SW_HIDE);
	}
	HWND GetHWND()
	{
		return m_hwnd;
	}
	virtual RenderEngine* GetCurrentRenderer();
	virtual void		DestroyRenderWindow() = 0;
	virtual BOOL		MouseLBDown(int x, int y) = 0;
	virtual BOOL		MouseLBUp(int x, int y) = 0;
	virtual BOOL		MouseRBDown(int x, int y) = 0;
	virtual BOOL		MouseRBUp(int x, int y) = 0;
	virtual BOOL		MouseMove(int x, int y) = 0;
	virtual BOOL KeyDown(WPARAM key) = 0;
	virtual void ProcessMenu(WORD command) = 0;

};