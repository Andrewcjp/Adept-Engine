
#include "InputController.h"


InputController::InputController()
{}


InputController::~InputController()
{}

void InputController::Update()
{
	memcpy(&LastButtonState, &ButtonState, GamePadButtons::Limit);
	memset(&ButtonState, 0, GamePadButtons::Limit);
	memset(&InputAxis, 0, sizeof(glm::vec2)*MaxControllerAxisNum);
	UpdateState();
}

glm::vec2 InputController::GetLeftThumbStickAxis() const
{
	return DeadZone(InputAxis[LeftThumbStickAxis], StickDeadZone);
}

glm::vec2 InputController::GetRightThumbStickAxis() const
{
	return DeadZone(InputAxis[RightThumbStickAxis], StickDeadZone);
}

bool InputController::GetButtonDown(GamePadButtons::Type button)
{
	return ButtonState[button] && !LastButtonState[button];
}

bool InputController::GetButton(GamePadButtons::Type button)
{
	return  ButtonState[button];
}

bool InputController::GetButtonUp(GamePadButtons::Type button)
{
	return  !ButtonState[button] && LastButtonState[button];
}

float InputController::DeadZone(float V, float Threshold)const
{
	return (abs(V) < Threshold ? 0 : V);
}

glm::vec2 InputController::DeadZone(const glm::vec2 V, float threshold)const
{
	return glm::vec2(DeadZone(V.x, threshold), DeadZone(V.y, threshold));
}

float InputController::GetLeftTriggerAxis() const
{
	return DeadZone(InputAxis[TriggerAxis].x, StickDeadZone);
}

float InputController::GetRightTriggerAxis() const
{
	return DeadZone(InputAxis[TriggerAxis].y, StickDeadZone);;
}

bool InputController::IsTrackingController() const
{
	return false;
}
