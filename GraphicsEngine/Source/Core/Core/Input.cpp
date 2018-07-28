#include "Input.h"
#include "RHI/RenderWindow.h"
#include "Physics/PhysicsEngine.h"
#include "Physics/RigidBody.h"
#include "Core/Engine.h"
#include "include\glm\glm.hpp"
#include "Rendering/Core/Camera.h"
#include "Core/GameObject.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "Editor\EditorWindow.h"
#include "Components\MeshRendererComponent.h"
#include "UI/UIManager.h"
#include "Core/Platform/PlatformCore.h"
Input* Input::instance = nullptr;

Input::Input()
{
	instance = this;
}

Input::~Input()
{

}

void Input::Clear()
{
	KeyMap.clear();

}
void Input::ProcessInput(const float)
{
	IsActiveWindow = PlatformWindow::IsActiveWindow();
	if (UIManager::GetCurrentContext() != nullptr)
	{
		return;//block input!
	}
}

bool Input::MouseLBDown(int, int)
{
	ShowCursor(false);
	return TRUE;
}

bool Input::MouseLBUp(int, int)
{
	ShowCursor(true);
	return TRUE;
}



bool Input::MouseMove(int, int, double)
{
	int height, width = 0;
	PlatformWindow::GetApplication()->GetDesktopResolution(height, width);
	int halfheight = (height / 2);
	int halfwidth = (width / 2);
	IntPoint Point = PlatformWindow::GetApplication()->GetMousePos();
	MouseAxis.x = (float)((halfheight)-(int)Point.x);
	MouseAxis.y = (float)(-((halfwidth)-(int)Point.y));
	Point.x = halfheight;
	Point.y = halfwidth;
	CentrePoint = Point;
	if (LockMouse)
	{
		PlatformWindow::GetApplication()->SetMousePos(CentrePoint);
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

bool Input::ProcessKeyDown(unsigned int key)
{
	//this only accounts for EN keyboard layouts
	const bool IsVKey = (int)key > 90;//vKey Start
	if (!IsVKey)
	{

		char c = PlatformWindow::GetApplication()->GetVirtualKeyAsChar(key);
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
		PlatformWindow::GetApplication()->SetMousePos(CentrePoint);
	}
	LockMouse = state;
}

void Input::ReciveMouseDownMessage(int Button, bool state)
{
	if (instance != nullptr)
	{
		if (MAX_MOUSE_BUTTON_COUNT > Button && Button >= 0)
		{
			instance->MouseKeyData[Button] = state;
		}
	}
}

bool Input::GetMouseButtonDown(int button)
{
	if (instance != nullptr)
	{
		if (MAX_MOUSE_BUTTON_COUNT > button && button >= 0)
		{
			return instance->MouseKeyData[button];
		}
	}
	return false;
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
	short key = PlatformWindow::GetApplication()->GetCharAsVirtualKey(c);
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
	return PlatformWindow::GetApplication()->IsKeyDown(key);
}

glm::vec2 Input::GetMouseInputAsAxis()
{
	if (instance != nullptr)
	{
		return instance->MouseAxis;
	}
	return glm::vec2();
}

