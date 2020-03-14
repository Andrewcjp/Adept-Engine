#pragma once
#ifdef PLATFORM_WINDOWS
#include "../../InputInterface.h"

class XInputController;
class InputController;
class XInputInterface : public InputInterface
{
public:
	XInputInterface();
	~XInputInterface();

	void Tick() override;
	InputController* GetController(int Index) override;
	int GetNumOfControllers() const override;
	void ShutDown() override;
	void CreateController();
private:
	std::vector<XInputController*> controllers;
};
#endif
