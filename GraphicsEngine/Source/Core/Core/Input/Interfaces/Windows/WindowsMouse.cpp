#include "WindowsMouse.h"
#include "UI/Core/UIWidget.h"
#include "Core/Input/Input.h"


WindowsMouse::WindowsMouse()
{
}


WindowsMouse::~WindowsMouse()
{
}

void WindowsMouse::ProcessMouseMSG(float wheelValue)
{
	MouseWheelDelta = wheelValue;
	const float DeltaMin = 0.5f;
	if (wheelValue > DeltaMin)
	{
		MouseWheelUpThisFrame = true;
	}
	if (wheelValue < -DeltaMin)
	{
		MouseWheelDownThisFrame = true;
	}
	UIInputEvent Event;
	Event.Pos = MousePosition;
	Event.ScrollAxis = MouseWheelDelta;
	Input::AddUIEvent(Event);
}
int MappingTable[]
{
	VK_LBUTTON,
	VK_RBUTTON,
	VK_MBUTTON
};
void WindowsMouse::UpdateState()
{
	for (int i = 0; i < MouseButton::Limit; i++)
	{
		ButtonState[i] = (GetKeyState(MappingTable[i]) & 0x8000);
	}
	IntPoint p = PlatformWindow::GetApplication()->GetMousePos();
	MousePosition = glm::ivec2(p.x,p.y);
}
