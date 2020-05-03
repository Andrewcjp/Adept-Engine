#pragma once
namespace MouseButton
{
	enum Type
	{
		ButtonLeft,
		ButtonRight,
		ButtonMiddle,
		Limit
	};
}
namespace InputButtonPressType
{
	enum Type
	{
		Press,
		Release,
		Hold,
		Limit
	};
}
class InputMouse
{
public:
	InputMouse();
	virtual ~InputMouse();
	void Update();

	bool GetButtonDown(MouseButton::Type button);
	bool GetButtonUp(MouseButton::Type button);
	bool GetButton(MouseButton::Type button);
	glm::vec2 GetMouseDeltaAxis() const { return MouseDeltaAxis; }
	float GetMouseWheelDelta() const { return MouseWheelDelta; }
	glm::ivec2 GetMousePosition() const { return MousePosition; }
	bool GetMouseWheelUp() const { return MouseWheelUpThisFrame; }
	bool GetMouseWheelDown() const { return MouseWheelDownThisFrame; }
protected:
	glm::ivec2 MousePosition;
	glm::vec2 MouseDeltaAxis;
	virtual void UpdateState() {};
	bool ButtonState[MouseButton::Limit] = {};
	bool LastButtonState[MouseButton::Limit] = {};
	float MouseWheelDelta = 0;
	bool MouseWheelUpThisFrame = false;
	bool MouseWheelDownThisFrame = false;
};

