#include "BaseApplication.h"

#if BUILD_WINDOW_ENGINE
//#include <Windows.h>
#include "RHI\RenderWindow.h"
#include "D3D11/D3D11Window.h"
#include "Resource.h"
#include <Windowsx.h>
#include <algorithm>
#include "TestGame.h"
#include "Physics\PhysicsEngine.h"
#include "Core\Engine.h"
#define TARGET_RESOLUTION 1         // 1-millisecond target resolution
BaseApplication* BaseApplication::s_oglapp = nullptr;
#pragma comment(lib, "winmm.lib")
BaseApplication::BaseApplication()
{ 
	m_appwnd = NULL;
	m_hInst = 0;
	m_terminate = false;
	m_engine = new Engine();
	TIMECAPS tc;
	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
	{
		// Error; application can't continue.
		MessageBoxA(0, "Timer Error Aborting", "Error", 0);
		Kill();
	}
}


BaseApplication::~BaseApplication()
{
	if (m_appwnd)
		delete m_appwnd;
}

BOOL BaseApplication::MyRegisterClass(HINSTANCE hinst)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = this->WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hinst;
	wcex.hIcon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_OGLWIN32));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"RenderWindow";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	if (!RegisterClassEx(&wcex))
		return FALSE;

	return TRUE;
}

BaseApplication* BaseApplication::CreateApplication(HINSTANCE hinst, LPSTR args, int nshow)
{
	if (!s_oglapp)
	{
		s_oglapp = new BaseApplication();

		s_oglapp->m_hInst = hinst;
		s_oglapp->MyRegisterClass(hinst);
		s_oglapp->m_engine->SetHInstWindow(hinst);
		//Now create an OGLWindow for this application
		s_oglapp->m_engine->SetGame(new TestGame());
		s_oglapp->m_engine->CreateApplication(hinst, args, nshow);
		
	}

	return s_oglapp;
}
HWND BaseApplication::GetHWND()
{
	return s_oglapp->GetApplicationWindow()->GetHWND();
}
void BaseApplication::DestroyApplication()
{
	if (s_oglapp)
		delete s_oglapp;

}

BaseApplication* BaseApplication::GetApplication()
{
	return s_oglapp;
}


int BaseApplication::Run()
{
	MSG msg = MSG();

	while (!m_terminate)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//peek for windows message
			if (msg.message == WM_QUIT)
			{
				if (IsRenderSwitch == true) {
					IsRenderSwitch = false;
				}
				else
				{
					Kill();
				}

			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if (m_engine->isWindowVaild && !m_terminate) {

			GetApplicationWindow()->Render();
		}
	}

	return (int)msg.wParam;
}

void BaseApplication::Kill()
{
	m_terminate = TRUE;
}

void BaseApplication::AddMenus(HWND hwnd)
{


	HMENU hMenu;
	HMENU hGOMenu = CreateMenu();
	HMENU hdebugMenu = CreateMenu();
	HMENU hRenderMenu = CreateMenu();
	HMENU hMenubar = CreateMenu();
	hMenu = CreateMenu();
	//file menu
	AppendMenuW(hMenu, MF_STRING, 5, L"&Save Scene");
	AppendMenuW(hMenu, MF_STRING, 6, L"&Load Scene");
	AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(hMenu, MF_STRING, IDM_FILE_QUIT, L"&Quit");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"&File");
#if BUILD_D3D11
	AppendMenuW(hRenderMenu, MF_UNCHECKED, IDM_RENDER_D3D11, L"&DirectX 11");
#endif
	AppendMenuW(hRenderMenu, MF_CHECKED, IDM_RENDER_OGL, L"&OpenGL");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hRenderMenu, L"&Renderer");
	//Gameobject menu
	AppendMenuW(hGOMenu, MF_STRING, 4, L"&Add GameObject ");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hGOMenu, L"&GameObjects");
	AppendMenuW(hdebugMenu, MF_STRING, 10, L"&Load DebugScene ");
	AppendMenuW(hdebugMenu, MF_STRING, 11, L"&Run Cook ");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hdebugMenu, L"&DEBUG");
	SetMenu(hwnd, hMenubar);
}

inline RenderWindow * BaseApplication::GetApplicationWindow()
{
	return m_engine->GetWindow();
}

LRESULT CALLBACK BaseApplication::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CREATE:
#if WITH_EDITOR
		s_oglapp->AddMenus(hwnd);
#endif
		break;
	case WM_COMMAND:

		switch (LOWORD(wparam)) {

		case IDM_RENDER_D3D11:
			s_oglapp->IsRenderSwitch = s_oglapp->m_engine->SwitchRenderAPI(RenderSystemD3D11);
			break;
		case IDM_RENDER_OGL:
			s_oglapp->IsRenderSwitch = s_oglapp->m_engine->SwitchRenderAPI(RenderSystemOGL);
			MessageBeep(MB_ICONINFORMATION);
			break;

		case IDM_FILE_QUIT:

			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		default:
			if (s_oglapp->m_engine->GetWindow())
			{
				s_oglapp->m_engine->GetWindow()->ProcessMenu(LOWORD(wparam));
			}
		}
		break;
	case WM_SIZE:
		s_oglapp->GetApplicationWindow()->Resize(LOWORD(lparam), HIWORD(lparam));
		break;
	case WM_CLOSE:
		s_oglapp->Kill();
		s_oglapp->GetApplicationWindow()->DestroyRenderWindow();
		break;
	case WM_MOUSEMOVE:
		s_oglapp->GetApplicationWindow()->MouseMove(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
		break;

	case WM_LBUTTONUP:
		s_oglapp->GetApplicationWindow()->MouseLBUp(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
		break;
	case WM_LBUTTONDOWN:
		s_oglapp->GetApplicationWindow()->MouseLBDown(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
		break;
	case WM_RBUTTONUP:
		s_oglapp->GetApplicationWindow()->MouseRBUp(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
		break;
	case WM_RBUTTONDOWN:
		s_oglapp->GetApplicationWindow()->MouseRBDown(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
		break;
	case WM_KEYDOWN:
		s_oglapp->GetApplicationWindow()->KeyDown(wparam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	return 0;
}
#endif