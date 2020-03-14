#include "XInputController.h"
#ifdef PLATFORM_WINDOWS
#include <Xinput.h>

XInputController::XInputController()
{}


XInputController::~XInputController()
{}

void XInputController::UpdateState()
{
	XINPUT_STATE State;
	ZeroMemory(&State, sizeof(XINPUT_STATE));
	if (XInputGetState(0, &State) == ERROR_SUCCESS)
	{
	   ButtonState[GamePadButtons::FaceButtonDown] = (State.Gamepad.wButtons & XINPUT_GAMEPAD_A);
	   ButtonState[GamePadButtons::FaceButtonRight] = (State.Gamepad.wButtons & XINPUT_GAMEPAD_B);
	   ButtonState[GamePadButtons::FaceButtonLeft] = (State.Gamepad.wButtons & XINPUT_GAMEPAD_X);
	   ButtonState[GamePadButtons::FaceButtonUp] = (State.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
	   ButtonState[GamePadButtons::LeftShoulder] = (State.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
	   ButtonState[GamePadButtons::RightShoulder] = (State.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

	   float normLX = fmaxf(-1, (float)State.Gamepad.sThumbLX / 32767);
	   float normLY = fmaxf(-1, (float)State.Gamepad.sThumbLY / 32767);
	   InputAxis[InputController::LeftThumbStickAxis] = glm::vec2(-normLX, -normLY);
	   float normRX = fmaxf(-1, (float)State.Gamepad.sThumbRX / 32767);
	   float normRY = fmaxf(-1, (float)State.Gamepad.sThumbRY / 32767);
	   InputAxis[InputController::RightThumbStickAxis] = glm::vec2(-normRX, -normRY);

	   float leftTrigger = (float)State.Gamepad.bLeftTrigger / 255;
	   float rightTrigger = (float)State.Gamepad.bRightTrigger / 255;
	   InputAxis[InputController::TriggerAxis] = glm::vec2(leftTrigger,rightTrigger);
	}
}
#endif