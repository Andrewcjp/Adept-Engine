#include "Input.h"
//#include <Windows.h>
#include "RHI/RenderWindow.h"
#include "Physics/PhysicsEngine.h"
#include "Physics/RigidBody.h"
#include "Core/Engine.h"
#include "include\glm\glm.hpp"
#include "Rendering/Core/Camera.h"
#include "Core/GameObject.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Rendering/Shaders/ShaderOutput.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "Editor\EditorWindow.h"
#include "Components\MeshRendererComponent.h"
#include "../UI/UIManager.h"
Input* Input::instance = nullptr;
HCURSOR Input::Cursor = NULL;
Input::Input(Camera* c, GameObject* playergo, HWND window, RenderWindow* wind)
{
	instance = this;
	playerGO = playergo;
	MainCam = c;
	m_hwnd = window;
	ogwindow = wind;
	OpenGlwindow = reinterpret_cast<EditorWindow*>(wind);
	Layout = GetKeyboardLayout(0);

}

Input::~Input()
{
	DestroyCursor(Cursor);
}

void Input::Clear()
{
	KeyMap.clear();
	IsTidleDown = false;
}
void Input::ProcessInput(const float)
{
	IsActiveWindow = (m_hwnd == GetActiveWindow());
	if (UIManager::GetCurrentContext() != nullptr)
	{
		return;//block input!
	}
}

BOOL Input::MouseLBDown(int, int)
{
	ShowCursor(false);
	return TRUE;
}

BOOL Input::MouseLBUp(int, int)
{
	ShowCursor(true);
	return TRUE;
}
void GetDesktopResolution(int& horizontal, int& vertical, HWND window)
{
	RECT desktop;
	// Get the size of screen to the variable desktop
	//	GetWindowRect(hDesktop, &desktop);
	GetClientRect(window, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right - desktop.left;
	vertical = desktop.bottom - desktop.top;
}
BOOL Input::MouseMove(int, int, double)
{

	int height, width = 0;
	GetDesktopResolution(height, width, m_hwnd);
	int halfheight = (height / 2);
	int halfwidth = (width / 2);

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(m_hwnd, &pt);
	MouseAxis.x = (float)((halfheight)-(int)pt.x);
	MouseAxis.y = (float)(-((halfwidth)-(int)pt.y));
	pt.x = halfheight;
	pt.y = halfwidth;
	ClientToScreen(m_hwnd, &pt);
	CentrePoint = pt;
	if (LockMouse)
	{
		SetCursorPos(pt.x, pt.y);
	}
	return TRUE;
}
void Input::SetSelectedObject(int index)
{
	instance->currentObjectIndex = index;
	if (instance->currentObjectIndex >= instance->ogwindow->GetCurrentRenderer()->GetObjects().size())
	{
		instance->currentObjectIndex = 0;
	}
	instance->Selectedobject = instance->ogwindow->GetCurrentRenderer()->GetObjects()[instance->currentObjectIndex];
}
void Input::ProcessQue()
{
	for (size_t i = 0; i < Inputque.size(); i++)
	{
		Inputque.front()->Func(this);
		Inputque.pop();
	}
}
BOOL Input::ProcessKeyDown(WPARAM key)
{

	InputEvent* ie = new InputEvent(this);
	switch (key)
	{
	case VK_ESCAPE:
		PostQuitMessage(0);
		break;
	case VK_F1:
		if (OpenGlwindow != nullptr)
		{
			if (OpenGlwindow->ShowHud)
			{
				OpenGlwindow->ShowHud = false;
			}
			else
			{
				OpenGlwindow->ShowHud = true;
			}
			OpenGlwindow->GetCurrentRenderer()->GetFilterShader()->SetFullScreen(!OpenGlwindow->ShowHud);
		}
		break;
	case VK_F2:
		if (OpenGlwindow != nullptr)
		{
			if (OpenGlwindow->ExtendedPerformanceStats)
			{
				OpenGlwindow->ExtendedPerformanceStats = false;
			}
			else
			{
				OpenGlwindow->ExtendedPerformanceStats = true;
			}
		}

		break;
	case VK_F3:
		if (OpenGlwindow != nullptr)
		{
			if (OpenGlwindow->ShowText)
			{
				OpenGlwindow->ShowText = false;
			}
			else
			{
				OpenGlwindow->ShowText = true;
			}
		}
		break;
	case VK_F4:

		if (FxAA)
		{
			FxAA = false;
		}
		else
		{
			FxAA = true;
		}
		if (Filters != nullptr)
		{
			Filters->SetFXAA(FxAA);
		}

		break;
	case VK_F5:
		if (main != nullptr)
		{
			ie->Func = &(Test);
			Inputque.push(ie);
		}
		break;
	case VK_F6:
		if (main != nullptr)
		{
			main->SetShadowVis();
		}
		break;
	case VK_F7:
		if (main != nullptr)
		{
			main->SetFullBright();
		}
		break;
	case VK_TAB:
		break;
	case VK_SPACE:
		break;
	case VK_DOWN:

		break;
	case VK_LEFT:
		break;
	case VK_NUMPAD0:
		currentObjectIndex++;
		if (currentObjectIndex >= ogwindow->GetCurrentRenderer()->GetObjects().size())
		{
			currentObjectIndex = 0;
		}
		Selectedobject = ogwindow->GetCurrentRenderer()->GetObjects()[currentObjectIndex];
		break;
	case VK_DECIMAL:
		currentObjectIndex--;
		if (currentObjectIndex < 0)
		{
			currentObjectIndex = 0;
		}
		Selectedobject = ogwindow->GetCurrentRenderer()->GetObjects()[currentObjectIndex];
		break;
	case VK_NUMPAD4:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(currentmoveamt, 0, 0));
		}
		break;
	case VK_NUMPAD6:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(-currentmoveamt, 0, 0));
		}
		break;
	case VK_NUMPAD8:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(0, 0, currentmoveamt));
		}
		break;
	case VK_NUMPAD5:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(0, 0, -currentmoveamt));
		}
		break;
	case VK_NUMPAD7:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(0, currentmoveamt, 0));
		}
		break;
	case VK_NUMPAD9:
		if (Selectedobject != nullptr)
		{
			Selectedobject->GetTransform()->TranslatePos(glm::vec3(0, -currentmoveamt, 0));
		}
		break;
	case VK_RIGHT:

		break;
	default:
		char c = (UINT)MapVirtualKey((UINT)key, MAPVK_VK_TO_CHAR);
		switch (c)
		{
		case '`'://w
			IsTidleDown = true;
			break;
		case 1:
			break;
		}
		KeyMap.emplace(c, true);
		return TRUE;
		break;
	}
	KeyMap.emplace((int)key, true);
	return TRUE;
}

void Input::LockCursor(bool state)
{
	//	ShowCursor(!state);
	if (state)
	{
		SetCursorPos(CentrePoint.x, CentrePoint.y);
	}
	LockMouse = state;
}

void Input::Test(Input * in)
{
	in->main->SetNormalVis();
}

bool Input::GetKeyDown(int c)
{
	if (instance != nullptr)
	{
		//todo: use array of keys?
		if (instance->KeyMap.find((int)c) != instance->KeyMap.end())
		{
			return instance->KeyMap.at((int)c);
		}
	}
	return false;
}

bool Input::GetKey(char c)
{
	if (instance == nullptr)
	{
		return false;
	}
	short key = VkKeyScanEx(c, instance->Layout);
	return GetVKey(key);
}

bool Input::GetVKey(short key)
{
	if (instance == nullptr)
	{
		return false;
	}
	if (!instance->IsActiveWindow)
	{
		return false;
	}
	if (GetKeyState(key) & 0x8000)
	{
		return true;
	}

	return false;
}

glm::vec2 Input::GetMouseInputAsAxis()
{
	if (instance != nullptr)
	{
		return instance->MouseAxis;
	}
	return glm::vec2();
}

