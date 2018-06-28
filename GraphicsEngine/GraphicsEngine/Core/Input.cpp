#include "Input.h"
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
#include "UI/UIManager.h"
Input* Input::instance = nullptr;
HCURSOR Input::Cursor = NULL;
Input::Input(HWND windowd)
{
	instance = this;
	m_hwnd = windowd;
	Layout = GetKeyboardLayout(0);

}

Input::~Input()
{
	DestroyCursor(Cursor);
}

void Input::Clear()
{
	KeyMap.clear();

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

void Input::ProcessQue()
{
	for (size_t i = 0; i < Inputque.size(); i++)
	{
		Inputque.front()->Func(this);
		Inputque.pop();
	}
}

bool Input::ProcessKeyDown(WPARAM key)
{
	const bool IsVKey = (int)key > 90;//vKey Start
	if (!IsVKey)
	{
		char c = (UINT)MapVirtualKey((UINT)key, MAPVK_VK_TO_CHAR);
		KeyMap.emplace(c, true);
		return true;
	}
	KeyMap.emplace((int)key, true);
	return true;
}

void Input::LockCursor(bool state)
{
	if (state)
	{
		SetCursorPos(CentrePoint.x, CentrePoint.y);
	}
	LockMouse = state;
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
	if (!instance->IsActiveWindow)
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

