#include "XInputInterface.h"
#ifdef PLATFORM_WINDOWS
#include "Core/Input/InputController.h"
#include "Core/Input/InputManager.h"
#include "XInputController.h"
#include <Xinput.h>


static ClassRegister<InputInterface, InputManager> Register([]() { return new XInputInterface(); });
XInputInterface::XInputInterface()
{
	CreateController();
}


XInputInterface::~XInputInterface()
{}

void XInputInterface::Tick()
{
	for (int i = 0; i < controllers.size(); i++)
	{
		controllers[i]->Update();
	}
}

InputController* XInputInterface::GetController(int Index)
{
	return (InputController*)controllers[Index];
}

int XInputInterface::GetNumOfControllers() const
{
	return (int)controllers.size();
}

void XInputInterface::ShutDown()
{
	
}
void XInputInterface::CreateController()
{
	controllers.push_back(new XInputController());
}
#endif