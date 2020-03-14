#pragma once
#ifdef PLATFORM_WINDOWS
#include "../../InputController.h"
class XInputController : public InputController
{
public:
	XInputController();
	virtual ~XInputController();
protected:
	void UpdateState() override;

};
#endif
