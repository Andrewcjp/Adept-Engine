#pragma once
#include "../RHI/RenderWindow.h"
#include "../D3D12/D3D12RHI.h"
class D3D12Window : public RenderWindow
{
public:
	D3D12Window();
	~D3D12Window();

	// Inherited via RenderWindow
	virtual bool CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen = false) override;
	virtual BOOL InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height) override;
	virtual void Render() override;
	virtual void Resize(int width, int height) override;
	virtual void DestroyRenderWindow() override;
	virtual BOOL MouseLBDown(int x, int y) override;
	virtual BOOL MouseLBUp(int x, int y) override;
	virtual BOOL MouseRBDown(int x, int y) override;
	virtual BOOL MouseRBUp(int x, int y) override;
	virtual BOOL MouseMove(int x, int y) override;
	virtual BOOL KeyDown(WPARAM key) override;
private:
	D3D12RHI* RHITest;
};

