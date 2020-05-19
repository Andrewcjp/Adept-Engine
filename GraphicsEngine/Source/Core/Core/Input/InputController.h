#pragma once
#include "Core/Transform.h"
//this is the interface between game code and hardware controllers.
//A controller might not support all functions E.g. tracking, touch pad etc.
//Each Implementation holds a list of button names that relate to the hardware specifics
//these can then be bound directly by the application (or the defaults used)
namespace GamePadButtons
{
	enum Type
	{
		FaceButtonDown,
		FaceButtonUp,
		FaceButtonLeft,
		FaceButtonRight,
		LeftShoulder,
		RightShoulder,
		Limit
	};
}
class InputController
{
public:
	InputController();
	virtual ~InputController();
	void Update();
	glm::vec2 GetLeftThumbStickAxis()const;
	glm::vec2 GetRightThumbStickAxis()const;
	static const int MaxControllerAxisNum = 3;
	static const int LeftThumbStickAxis = 0;
	static const int RightThumbStickAxis = 1;
	static const int TriggerAxis = 2;
	bool GetButtonDown(GamePadButtons::Type button);
	bool GetButton(GamePadButtons::Type button);
	bool GetButtonUp(GamePadButtons::Type button);
	float DeadZone(const float V, float Threshold) const;
	glm::vec2 DeadZone(const glm::vec2 V, float threshold)const;
	float GetLeftTriggerAxis()const;
	float GetRightTriggerAxis()const;
	//Is this a VR controller?
	virtual bool IsTrackingController()const;
	Transform m_ControllerTransform;
protected:
	float StickDeadZone = 0.1f;
	//collect button data from HW
	virtual void UpdateState() {};
	glm::vec2 InputAxis[MaxControllerAxisNum] = {};
	bool ButtonState[GamePadButtons::Limit] = {};
	bool LastButtonState[GamePadButtons::Limit] = {};
};

