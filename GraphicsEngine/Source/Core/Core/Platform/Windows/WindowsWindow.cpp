
#include "WindowsWindow.h"
#ifdef PLATFORM_WINDOWS
#include "Core/Engine.h"
#include "RHI/RenderWindow.h"
#include <Windowsx.h>
#include "Resource.h"
#include "Core/Components/ComponentRegistry.h"
#include "Core/EngineTypes.h"
#include "Core/Input/Input.h"
#include "Core/Platform/Windows/WindowsApplication.h"
#include <timeapi.h>
#include "WinLauncher.h"
#include "Core/Version.h"
#include "Core/Utils/StringUtil.h"
#include "Rendering/Core/Screen.h"
#include <commctrl.h>



WindowsWindow* WindowsWindow::app = nullptr;
WindowsWindow::WindowsWindow()
{
	m_hInst = 0;
	m_terminate = false;
	TIMECAPS tc;
	Layout = GetKeyboardLayout(0);
	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
	{
		// Error; application can't continue.
		MessageBoxA(0, "Timer Error Aborting", "Error", 0);
		Kill(0);
	}
	Cursor = CopyCursor(LoadCursor(NULL, IDC_ARROW));
	Cursor = SetCursor(Cursor);
	Cursors[GenericWindow::CursorType::IBeam] = LoadCursor(NULL, IDC_IBEAM);
	Cursors[GenericWindow::CursorType::Normal] = LoadCursor(NULL, IDC_ARROW);
}


WindowsWindow::~WindowsWindow()
{
	DestroyCursor(Cursor);
}

BOOL WindowsWindow::MyRegisterClass(HINSTANCE hinst)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = this->WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hinst;
	wcex.hIcon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"RenderWindow";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	if (!RegisterClassEx(&wcex))
	{
		return FALSE;
	}

	return TRUE;
}

WindowsWindow* WindowsWindow::CreateApplication(Engine* EnginePtr, HINSTANCE hinst, LPSTR args, int nshow)
{
	if (!app)
	{
		app = new WindowsWindow();
		app->m_engine = EnginePtr;
		app->m_hInst = hinst;
		app->MyRegisterClass(hinst);
		//Now create an OGLWindow for this application
		if (!app->m_engine->GetIsCooking())
		{
			app->CreateSplashWindow();
			app->CreateOSWindow(Screen::GetWindowWidth(), Screen::GetWindowHeight());
			app->m_engine->CreateApplication();
			app->SetVisible(true);
			app->SetupHPMI();
		}
	}
	return app;
}

void WindowsWindow::SetVisible(bool visible)
{
	ShowWindow(HWindow, visible ? SW_SHOW : SW_HIDE);
}

void WindowsWindow::Maximize()
{
	ShowWindow(app->HWindow, SW_MAXIMIZE);
}

bool WindowsWindow::CreateOSWindow(int width, int height)
{
#define NOTITLEWINDOW 0
	std::string Title = std::string(ENGINE_NAME) + " " + Version::GetFullVersionString();
	app->HWindow = CreateWindowEx(
		0,
		L"RenderWindow", StringUtils::ConvertStringToWide(Title).c_str(),
#if !NOTITLEWINDOW
		WS_OVERLAPPEDWINDOW |
#else
		WS_OVERLAPPED |
#endif
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, width, height, NULL, NULL, app->m_hInst, NULL);
#if NOTITLEWINDOW
	SetWindowLong(app->HWindow, GWL_STYLE, 0);
	glm::ivec2 Offset = glm::ivec2(100, 150);
	SetWindowPos(app->HWindow, 0, Offset.x, Offset.y, Offset.x + width, Offset.y + height, SWP_FRAMECHANGED);
#endif
	//SetVisible(true);
	return true;
}

HWND WindowsWindow::GetHWND()
{
	return app->HWindow;
}

HINSTANCE WindowsWindow::GetHInstance()
{
	return app->m_hInst;
}
void WindowsWindow::DestroyApplication()
{
	if (app != nullptr)
	{
		DestroyWindow(app->HWindow);
		SafeDelete(app);
	}
}

WindowsWindow* WindowsWindow::GetApplication()
{
	return app;
}


int WindowsWindow::Run()
{
	MSG msg = MSG();
	bool DidJustBoot = Engine::IsSecondLoad();
	while (!m_terminate)
	{
		if (Input::Get())
		{
			Input::Get()->ResetMouse();
		}
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//peek for windows message
			if (msg.message == WM_QUIT)
			{
				if (DidJustBoot)
				{
					//if the app just restarted ignore the kill from the last run
					DidJustBoot = false;
					ensure(false);
					continue;
				}
				Kill(0);
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if (m_engine->GetWindowValid() && !m_terminate)
		{
			m_engine->OnRender();
		}
	}
	app->m_engine->OnDestoryWindow();
	//clear queue
	return (int)msg.wParam;
}

void WindowsWindow::EmptyMessageQueue()
{
	MSG msg = MSG();
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void WindowsWindow::Kill(int code)
{
	app->m_terminate = TRUE;
	if (code != Engine::RestartCode)
	{
		Engine::GetEPD()->Restart = false;
	}
}

void WindowsWindow::CreateSplashWindow()
{
#if WITH_EDITOR
	if (app == nullptr)
	{
		return;
	}
	int width = 400;
	int height = 100;
	std::string Title = std::string(ENGINE_NAME) + " " + Version::GetFullVersionString();
	app->SplashWindow = CreateWindowEx(
		0,
		L"RenderWindow", StringUtils::ConvertStringToWide(Title).c_str(),
		WS_OVERLAPPED, 0, 0, width, height, NULL, NULL, app->m_hInst, NULL);
	Label = CreateWindowEx(0, L"STATIC", (LPTSTR)NULL,
		WS_CHILD | WS_VISIBLE | SS_CENTER,
		0, 0, width, height,
		app->SplashWindow, (HMENU)0, m_hInst, NULL);
	SetWindowText(Label, L"Loading: 0%");

	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	HMONITOR g = MonitorFromWindow(SplashWindow, MONITOR_DEFAULTTONEAREST);
	GetMonitorInfo(g, &info);
	glm::ivec2 offset = glm::ivec2();
	offset.x = (info.rcWork.right / 2) - width / 2;
	offset.y = (info.rcWork.bottom / 2) - height / 2;
	SetWindowPos(app->SplashWindow, 0, offset.x, offset.y, width, height, SWP_FRAMECHANGED);
	ShowWindow(SplashWindow, SW_SHOW);
#endif	
}

void WindowsWindow::TickSplashWindow(int amt /*= 1*/, std::string Section /*= std::string()*/)
{
#if WITH_EDITOR
	if (app == nullptr)
	{
		return;
	}
	//process messages
	EmptyMessageQueue();
	app->CurrentProgress += amt;
	if (amt == -1)
	{
		app->CurrentProgress = 100;
	}
	//update window
	SetWindowText(app->Label, (std::wstring(L"Loading: ") + std::to_wstring(app->CurrentProgress) + L"% \n" + StringUtils::ConvertStringToWide(Section)).c_str());
#endif
}

void WindowsWindow::DestorySplashWindow()
{
#if WITH_EDITOR
	if (app == nullptr)
	{
		return;
	}
	ShowWindow(app->SplashWindow, SW_HIDE);
	DestroyWindow(app->SplashWindow);
	app->EmptyMessageQueue();
#endif
}

void WindowsWindow::GetDesktopResolution(int & horizontal, int & vertical)
{
	RECT desktop;
	// Get the size of screen to the variable desktop
	//	GetWindowRect(hDesktop, &desktop);
	GetClientRect(HWindow, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right - desktop.left;
	vertical = desktop.bottom - desktop.top;
}

IntPoint WindowsWindow::GetMousePos()
{
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(HWindow, &pt);
	return IntPoint(pt.x, pt.y);
}

void WindowsWindow::SetMousePos(IntPoint & point)
{
	POINT pt = { point.x,point.y };
	ClientToScreen(HWindow, &pt);
	SetCursorPos(pt.x, pt.y);
}

char WindowsWindow::GetVirtualKeyAsChar(unsigned int key)
{
	return (char)MapVirtualKey((UINT)key, MAPVK_VK_TO_CHAR);
}

short WindowsWindow::GetCharAsVirtualKey(char c)
{
	return VkKeyScanEx(c, Layout);
}

bool WindowsWindow::IsKeyDown(short key)
{
	if (::GetAsyncKeyState(key) & 0x8000)
	{
		return true;
	}
	return false;
}

void WindowsWindow::SetCursorType(GenericWindow::CursorType Type)
{
	switch (Type)
	{
		case GenericWindow::CursorType::IBeam:
			SetCursor(GetApplication()->Cursors[GenericWindow::CursorType::IBeam]);
			break;
		default:
		case GenericWindow::CursorType::Drag:
		case GenericWindow::CursorType::Normal:
			SetCursor(GetApplication()->Cursors[GenericWindow::CursorType::Normal]);
			break;
	}
}


bool WindowsWindow::IsActiveWindow()
{
	return (WindowsWindow::GetHWND() == GetActiveWindow());
}
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif
void WindowsWindow::SetupHPMI()
{
	if (Input::Get()->IsUsingHPMI())
	{
		RAWINPUTDEVICE Rid;
		Rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid.usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid.dwFlags = 0;// RIDEV_INPUTSINK;
		Rid.hwndTarget = GetHWND();
		ensure(::RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE)));
	}
}

LRESULT CALLBACK WindowsWindow::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_CREATE:
			break;
		case WM_SIZE:
			if (app->m_engine->GetRenderWindow())
			{
				const int Width = LOWORD(lparam);
				const int Height = HIWORD(lparam);
				if (Screen::NeedsWindowUpdate(Width, Height))
				{
					Screen::Resize(Width, Height);
					app->m_engine->GetRenderWindow()->Resize(Width, Height);
					RHI::Get()->ResizeSwapChain(Width, Height);
				}
			}
			break;
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			if (app->m_engine->GetRenderWindow())
			{
				app->m_engine->GetRenderWindow()->MouseMove(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			}
			break;

		case WM_LBUTTONUP:
			app->m_engine->GetRenderWindow()->MouseLBUp(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			Input::ReciveMouseDownMessage(0, false);
			break;
		case WM_LBUTTONDOWN:
			app->m_engine->GetRenderWindow()->MouseLBDown(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			Input::ReciveMouseDownMessage(0, true);
			break;
		case WM_RBUTTONUP:
			app->m_engine->GetRenderWindow()->MouseRBUp(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			Input::ReciveMouseDownMessage(1, false);
			break;
		case WM_RBUTTONDOWN:
			app->m_engine->GetRenderWindow()->MouseRBDown(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			Input::ReciveMouseDownMessage(1, true);
			break;
		case WM_KEYDOWN:
		{
			//From MSDN The repeat count for the current message. https://docs.microsoft.com/en-us/windows/desktop/inputdev/wm-keydown
			// LPARAM bit 30 will be ZERO for new presses, or ONE if this is a repeat
			bool bIsRepeat = (lparam & 0x40000000) != 0;
			if (!bIsRepeat)
			{
				app->m_engine->HandleInput(LOWORD(wparam));
			}
			break;
		}
		case WM_KEYUP:
		{
			//key up doesn't repeat!
			app->m_engine->HandleKeyUp(LOWORD(wparam));
			break;
		}
		case WM_INPUT:
		{
			UINT dwSize = 40;
			static BYTE lpb[40];

			GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				int xPosRelative = raw->data.mouse.lLastX;
				int yPosRelative = raw->data.mouse.lLastY;
				if (Input::Get())
				{
					Input::Get()->ReciveMouseAxisData(glm::vec2(xPosRelative, yPosRelative));
				}
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			const float SpinFactor = 1 / 120.0f;
			const short WheelDelta = GET_WHEEL_DELTA_WPARAM(wparam);

			if (Input::Get())
			{
				Input::Get()->ProcessMouseWheel(static_cast<float>(WheelDelta)*SpinFactor);
			}
		}
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