#pragma once
class RenderEngine;
class RenderWindow
{
public:
	RenderWindow()
	{}
	virtual				~RenderWindow()
	{}

	virtual bool			CreateRenderWindow(int width, int height) = 0;

	virtual void			Render() = 0;
	virtual void			Resize(int width, int height, bool force = false) = 0;

	virtual RenderEngine*	GetCurrentRenderer();
	virtual void			DestroyRenderWindow() = 0;
	virtual void			OnWindowContextLost() = 0;
};