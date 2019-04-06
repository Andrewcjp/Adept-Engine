#include "Input.h"
#include "RHI/RenderWindow.h"
#include "Core/Engine.h"
#include "UI/UIManager.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Windows/WindowsWindow.h"
#include "Editor/EditorWindow.h"
Input* Input::instance = nullptr;

void Input::Startup()
{
	instance = new Input();

}

void Input::ShutDown()
{
	SafeDelete(instance);
}

void Input::ReciveMouseAxisData(glm::vec2 data)
{
	if (IsUsingHPMI())
	{
		MouseAxis = data;
	}
}

Input::Input()
{
	UseHighPrecisionMouseInput = false;
	//ProcessInput();
}

Input::~Input()
{}

Input * Input::Get()
{
	return instance;
}

void Input::Clear()
{
	KeyMap.clear();
	MouseWheelUpThisFrame = false;
	MouseWheelDownThisFrame = false;
	DidJustPause = false;
}

void Input::ForceClear()
{
	Clear();
	for (int i = 0; i < MAX_MOUSE_BUTTON_COUNT; i++)
	{
		instance->MouseKeyData[i] = false;
	}
	DidJustPause = true;
}

bool Input::GetMouseWheelUp()
{
	return Get()->MouseWheelUpThisFrame;
}

bool Input::GetMouseWheelDown()
{
	return Get()->MouseWheelDownThisFrame;
}

void Input::ResetMouse()
{
	MouseAxis = glm::vec2();
	MouseSampleCount = 0;
}

void Input::ProcessInput()
{
	bool PreviousValue = IsActiveWindow;
	IsActiveWindow = PlatformWindow::IsActiveWindow();
	if (!IsActiveWindow && PreviousValue)
	{
#if !_DEBUG && !WITH_EDITOR 
		Engine::Get()->GetRenderWindow()->OnWindowContextLost();
#endif
	}
	if (UIManager::GetCurrentContext() != nullptr)
	{
		return;//block input!
	}
}

bool Input::MouseLBDown(int, int)
{
	return TRUE;
}

bool Input::MouseLBUp(int, int)
{
	return TRUE;
}

bool Input::MouseMove(int x, int y, double)
{
	if (!PlatformWindow::IsActiveWindow())
	{
		return false;
	}
	//if (CurrentFrame != RHI::GetFrameCount())
	{
		MousePosScreen = IntPoint(x, y);
		int height, width = 0;
		PlatformWindow::GetApplication()->GetDesktopResolution(height, width);
		int halfheight = (height / 2);
		int halfwidth = (width / 2);
		IntPoint Point = PlatformWindow::GetApplication()->GetMousePos();
		if (!IsUsingHPMI())
		{
			MouseAxis.x += (float)(halfheight - (float)Point.x);
			MouseAxis.y += -(float)(halfwidth - (float)Point.y);
			MouseSampleCount++;
		}
		Point.x = halfheight;
		Point.y = halfwidth;
		CentrePoint = Point;
		//Log::LogMessage(std::to_string(RHI::GetFrameCount()) + "  " + glm::to_string(instance->MouseAxis) + "  x" + std::to_string(MousePosScreen.x) + "  y" + std::to_string(MousePosScreen.y));
		CurrentFrame = RHI::GetFrameCount();
	}
	if (LockMouse)
	{
		PlatformWindow::GetApplication()->SetMousePos(CentrePoint);
	}
	return TRUE;
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

bool Input::ProcessKeyUp(unsigned int key)
{
	//this only accounts for EN keyboard layouts
	const bool IsVKey = (int)key > 90;//vKey Start
	if (!IsVKey)
	{
		char c = PlatformWindow::GetApplication()->GetVirtualKeyAsChar(key);
		KeyMap.emplace(c, false);
		return true;
	}
	KeyMap.emplace((int)key, false);
	return true;
}

void Input::ProcessMouseWheel(float Delta)
{
	const float DeltaMin = 0.5f;
	if (Delta > DeltaMin)
	{
		MouseWheelUpThisFrame = true;
	}
	if (Delta < -DeltaMin)
	{
		MouseWheelDownThisFrame = true;
	}
}


void Input::SetCursorState(bool Locked, bool Visible)
{
#if WITH_EDITOR
	if (!EditorWindow::GetInstance()->IsEditorEjected())
	{
		LockCursor(Locked);
		SetCursorVisible(Visible);
	}
	else
	{
		LockCursor(false);
		SetCursorVisible(true);
	}
#else
	LockCursor(Locked);
	SetCursorVisible(Visible);
#endif
}

void Input::LockCursor(bool state)
{
	if (state && PlatformWindow::IsActiveWindow())
	{
		PlatformWindow::GetApplication()->SetMousePos(instance->CentrePoint);
	}
	instance->LockMouse = state;
}

void Input::ReciveMouseDownMessage(int Button, bool state)
{
	if (instance->DidJustPause)
	{
		return;
	}
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

void Input::SetCursorVisible(bool state)
{
	int CurrentCount = ShowCursor(state);
	if (CurrentCount > 1)
	{
		CurrentCount = ShowCursor(false);
	}
	if (CurrentCount < -1)
	{
		CurrentCount = ShowCursor(true);
	}
}

IntPoint Input::GetMousePos()
{
	if (instance != nullptr)
	{
		return instance->MousePosScreen;
	}
	return IntPoint();
}

bool Input::GetKeyDown(int c)
{
	if (instance != nullptr)
	{
		//#Input use array of keys?
		if (instance->KeyMap.find((int)c) != instance->KeyMap.end())
		{
			return instance->KeyMap.at((int)c);
		}
	}
	return false;
}
//A key is in the keymap only if we recived an event for it this frame so false is key up in this case.
bool Input::GetKeyUp(int c)
{
	if (instance != nullptr)
	{
		//#Input: use array of keys?
		if (instance->KeyMap.find((int)c) != instance->KeyMap.end())
		{
			return !instance->KeyMap.at((int)c);
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
	if (UIManager::GetCurrentContext() != nullptr)
	{
		return false;
	}
	return PlatformWindow::GetApplication()->IsKeyDown(key);
}

glm::vec2 Input::GetMouseInputAsAxis()
{
	if (instance != nullptr)
	{
		if (glm::length2(instance->MouseAxis) > 0.0f)
		{
			return instance->MouseAxis / std::min(instance->MouseSampleCount, 1);
		}
		return instance->MouseAxis;
	}
	return glm::vec2();
}

