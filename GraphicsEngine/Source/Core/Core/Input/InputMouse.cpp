#include "Stdafx.h"
#include "InputMouse.h"
#include "Input.h"


InputMouse::InputMouse()
{
}


InputMouse::~InputMouse()
{
}

void InputMouse::Update()
{
	MouseWheelUpThisFrame = false;
	MouseWheelDownThisFrame = false;
	memcpy(&LastButtonState, &ButtonState, MouseButton::Limit);
	memset(&ButtonState, 0, MouseButton::Limit);
	MouseDeltaAxis = glm::vec2(0, 0);
	UpdateState();

	int height, width = 0;
	PlatformWindow::GetApplication()->GetDesktopResolution(height, width);
	int halfheight = (height / 2);
	int halfwidth = (width / 2);
	MouseDeltaAxis.x = (float)(halfheight - (float)MousePosition.x);
	MouseDeltaAxis.y = -(float)(halfwidth - (float)MousePosition.y);
	if (Input::SendInputEvents())
	{
		for (int i = 0; i < MouseButton::Limit; i++)
		{
			if (GetButtonDown((MouseButton::Type)i))
			{
				UIInputEvent Event;
				Event.Pos = MousePosition;
				Event.Mouse = (MouseButton::Type)i;
				Event.PressType = InputButtonPressType::Press;
				Input::AddUIEvent(Event);
			}
			if (GetButtonUp((MouseButton::Type)i))
			{
				UIInputEvent Event;
				Event.Pos = MousePosition;
				Event.Mouse = (MouseButton::Type)i;
				Event.PressType = InputButtonPressType::Release;
				Input::AddUIEvent(Event);
			}
		}
	}
}

bool InputMouse::GetButtonDown(MouseButton::Type button)
{
	return ButtonState[button] && !LastButtonState[button];
}
bool InputMouse::GetButtonUp(MouseButton::Type button)
{
	return !ButtonState[button] && LastButtonState[button];
}
bool InputMouse::GetButton(MouseButton::Type button)
{
	return ButtonState[button];
}