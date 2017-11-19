#include "stdafx.h"
#include "D3D12Window.h"


D3D12Window::D3D12Window()
{
}


D3D12Window::~D3D12Window()
{
}

bool D3D12Window::CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen)
{
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		L"RenderWindow", L"OGLWindow", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, width, height, NULL, NULL, hInstance, NULL);
	InitWindow(NULL, m_hwnd, NULL, width, height);
	return true;
}

BOOL D3D12Window::InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height)
{

	m_width = width;
	m_height = height;
	RHITest = new D3D12RHI();
	RHITest->m_height = m_height;
	RHITest->m_width = m_width;
	RHITest->m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	RHITest->LoadPipeLine();
	RHITest->LoadAssets();
	return 1;
}

void D3D12Window::Render()
{
	RHITest->OnRender();
}

void D3D12Window::Resize(int width, int height)
{

}

void D3D12Window::DestroyRenderWindow()
{
	RHITest->OnDestroy();
	delete RHITest;
}

BOOL D3D12Window::MouseLBDown(int x, int y)
{
	return 0;
}

BOOL D3D12Window::MouseLBUp(int x, int y)
{
	return 0;
}

BOOL D3D12Window::MouseRBDown(int x, int y)
{
	return 0;
}

BOOL D3D12Window::MouseRBUp(int x, int y)
{
	return 0;
}

BOOL D3D12Window::MouseMove(int x, int y)
{
	return 0;
}

BOOL D3D12Window::KeyDown(WPARAM key)
{
	return 0;
}
