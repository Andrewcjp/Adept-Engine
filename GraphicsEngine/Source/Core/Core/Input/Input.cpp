#include "Input.h"
#include "RHI/RenderWindow.h"
#include "Core/Engine.h"
#include "UI/UIManager.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Windows/WindowsWindow.h"
#include "Editor/EditorWindow.h"
#include "InputManager.h"
#include "InputKeyboard.h"
#include "InputMouse.h"
#include "TextInputHandler.h"

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
	IManager = new InputManager();
	IManager->InitInterfaces();
	m_pTextHandler = new TextInputHandler();
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
	DidJustPause = false;
}

void Input::ForceClear()
{
	Clear();
	DidJustPause = true;
}

bool Input::GetMouseWheelUp()
{
	return GetInputManager()->GetMouse(0)->GetMouseWheelUp();
}

bool Input::GetMouseWheelDown()
{
	return GetInputManager()->GetMouse(0)->GetMouseWheelDown();
}

void Input::ResetMouse()
{
	MouseAxis = glm::vec2();
	MouseSampleCount = 0;
}

InputManager * Input::GetInputManager()
{
	return instance->IManager;
}

bool Input::SendInputEvents()
{
	return true;
}

void Input::AddUIEvent(UIInputEvent Event)
{
	instance->Events.push_back(Event);
}

bool Input::CanReceiveInput(EInputChannel::Type Channel)
{
	if (instance->GetCurrentChannel() != Channel && Channel != EInputChannel::Global)
	{
		return false;
	}
	if (Channel == EInputChannel::Game)
	{
		if (UIManager::Get()->IsUIBlocking())//put this on a flag!
		{
			return false;
		}
	}
	if (Channel != EInputChannel::TextInput && Channel != EInputChannel::Global)
	{
		if (TextInputHandler::Get()->IsActive())
		{
			return false;//block input!
		}
	}
	return true;
}

void Input::SetInputChannel(EInputChannel::Type Channel)
{
	instance->m_CurrentChannel = Channel;
}

void Input::ProcessInput()
{
	IManager->Tick();
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

bool Input::MouseMove(int x, int y)
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
	return true;
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

bool Input::GetMouseButtonDown(MouseButton::Type button, EInputChannel::Type Channel/* = EInputChannel::Game*/)
{
	if (!CanReceiveInput(Channel))
	{
		return false;
	}
	return GetInputManager()->GetMouse(0)->GetButtonDown(button);
}

float Input::GetMouseWheelAxis()
{
	return GetInputManager()->GetMouse()->GetMouseWheelDelta();
}

bool Input::GetMouseButton(MouseButton::Type button, EInputChannel::Type Channel)
{
	if (!CanReceiveInput(Channel))
	{
		return false;
	}
	return GetInputManager()->GetMouse(0)->GetButton(button);
}

void Input::SetCursorVisible(bool state)
{
#ifdef PLATFORM_WINDOWS
	int CurrentCount = ShowCursor(state);
	if (CurrentCount > 1)
	{
		CurrentCount = ShowCursor(false);
	}
	if (CurrentCount < -1)
	{
		CurrentCount = ShowCursor(true);
	}
#endif
}

IntPoint Input::GetMousePos()
{
	if (instance != nullptr)
	{
		return instance->MousePosScreen;
	}
	return IntPoint();
}

bool Input::GetKeyDown(KeyCode::Type key, EInputChannel::Type Channel/* = EInputChannel::Game*/)
{
	if (!CanReceiveInput(Channel))
	{
		return false;
	}
	return GetInputManager()->GetKeyboard(0)->IsKeyDown(key);
}

bool Input::GetKey(KeyCode::Type key, EInputChannel::Type Channel/* = EInputChannel::Game*/)
{
	if (!CanReceiveInput(Channel))
	{
		return false;
	}
	return GetInputManager()->GetKeyboard(0)->IsKey(key);
}

bool Input::GetKeyUp(KeyCode::Type key, EInputChannel::Type Channel/* = EInputChannel::Game*/)
{
	if (!CanReceiveInput(Channel))
	{
		return false;
	}
	return GetInputManager()->GetKeyboard(0)->IsKeyUp(key);
}


glm::vec2 Input::GetMouseInputAsAxis()
{
	//if (GetInputManager() && GetInputManager()->GetMouse(0))
	//{
	//	return GetInputManager()->GetMouse(0)->GetMouseDeltaAxis();
	//}
	if (instance != nullptr)
	{
		if (glm::length2(instance->MouseAxis) > 0.0f)
		{
			return instance->MouseAxis / std::max(instance->MouseSampleCount, 1);
		}
		return instance->MouseAxis;
	}
	return glm::vec2();
}

