#pragma once
#include "Core/Input/InputMouse.h"
class WindowsMouse: public InputMouse
{
public:
	WindowsMouse();
	~WindowsMouse();
	void ProcessMouseMSG(float wheelValue);
protected:
	void UpdateState() override;

};

