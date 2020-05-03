#include "WindowsInputInterface.h"
#include "WindowsMouse.h"
#include "WindowsKeyboard.h"
#include "Core/Input/InputManager.h"
WindowsInputInterface* WindowsInputInterface::Instance = nullptr;
static ClassRegister<InputInterface, InputManager> Register([]() { return new WindowsInputInterface(); });
WindowsInputInterface::WindowsInputInterface()
{
	Keyboards.push_back(new WindowsKeyboard());
	Mice.push_back(new WindowsMouse());
	Instance = this;
}

WindowsInputInterface::~WindowsInputInterface()
{

}

void WindowsInputInterface::Tick()
{
	for (int i = 0; i < Keyboards.size(); i++)
	{
		Keyboards[i]->Update();
	}
	for (int i = 0; i < Mice.size(); i++)
	{
		Mice[i]->Update();
	}
}

int WindowsInputInterface::GetNumOfControllers() const
{
	return 0;
}

InputController* WindowsInputInterface::GetController(int Index)
{
	return nullptr;
}

int WindowsInputInterface::GetNumOfKeyBoards() const
{
	return Keyboards.size();
}

InputKeyboard* WindowsInputInterface::GetKeyboard(int Index)
{
	return Keyboards[Index];
}

int WindowsInputInterface::GetNumOfMice() const
{
	return Mice.size();
}

InputMouse* WindowsInputInterface::GetMouse(int Index)
{
	return Mice[Index];
}


void WindowsInputInterface::ShutDown()
{
	
}

void WindowsInputInterface::ProcessMouseMSG(float wheelValue)
{
	for (int i = 0; i < Mice.size(); i++)
	{
		Mice[i]->ProcessMouseMSG(wheelValue);
	}
}

