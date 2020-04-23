#include "InputInterface.h"


InputInterface::InputInterface()
{}


InputInterface::~InputInterface()
{}

void InputInterface::Tick()
{}

InputController * InputInterface::GetController(int Index)
{
	return nullptr;
}

int InputInterface::GetNumOfControllers() const
{
	return 0;
}

int InputInterface::GetNumOfKeyBoards()const
{
	return 0;
}

InputKeyboard * InputInterface::GetKeyboard(int Index)
{
	return nullptr;
}

int InputInterface::GetNumOfMice() const
{
	return 0;
}

InputMouse * InputInterface::GetMouse(int Index)
{
	return nullptr;
}

bool InputInterface::HasHMD() const
{
	return false;
}

void InputInterface::ShutDown()
{

}
