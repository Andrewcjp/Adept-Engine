#pragma once
class RenderEngine;
class RenderWindow
{
protected:
	int			m_width;
	int			m_height;
public:
	RenderWindow() {}
	virtual				~RenderWindow() {}

	virtual bool			CreateRenderWindow( int width, int height) =0;

	virtual void			Render() = 0;
	virtual void			Resize(int width, int height) = 0;
	
	virtual RenderEngine*	GetCurrentRenderer();
	virtual void			DestroyRenderWindow() = 0;
	virtual bool			MouseLBDown(int x, int y) = 0;
	virtual bool			MouseLBUp(int x, int y) = 0;
	virtual bool			MouseRBDown(int x, int y) = 0;
	virtual bool			MouseRBUp(int x, int y) = 0;
	virtual bool			MouseMove(int x, int y) = 0;
	virtual void			ProcessMenu(unsigned short command) = 0;
	virtual void			OnWindowContextLost() = 0;
};