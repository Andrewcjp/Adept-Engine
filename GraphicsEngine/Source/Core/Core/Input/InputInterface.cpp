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

bool InputInterface::HasHMD() const
{
	return false;
}

void InputInterface::ShutDown()
{

}
