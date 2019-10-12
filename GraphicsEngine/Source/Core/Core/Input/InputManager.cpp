
#include "InputManager.h"
#include "InputInterface.h"
#include "Interfaces/SteamVR/SteamVRInputInterface.h"

InputManager::InputManager()
{}


InputManager::~InputManager()
{}

void InputManager::InitInterfaces()
{
#if BUILD_STEAMVR
	if (SteamVRInputInterface::CanInit())
	{
		VrInterface = new SteamVRInputInterface();
		Interfaces.push_back(VrInterface);
	}
#endif
}

void InputManager::Tick()
{
	for (int i = 0; i < Interfaces.size(); i++)
	{
		Interfaces[i]->Tick();
	}
}

void InputManager::EnumConnectedDevices()
{

}

InputInterface * InputManager::GetActiveVRInterface()
{
	return VrInterface;
}

InputController* InputManager::GetController(int index, int interfaceindex /*= -1*/)
{
	if (interfaceindex != -1)
	{
		return Interfaces[interfaceindex]->GetController(index);
	}
	int Accum = 0;
	for (int i = 0; i < Interfaces.size(); i++)
	{
		if (Accum + Interfaces[i]->GetNumOfControllers() >= index)
		{
			return Interfaces[i]->GetController(index - Accum);
		}
		Accum += Interfaces[i]->GetNumOfControllers();
	}
	return nullptr;
}
