
#include "InputManager.h"
#include "InputInterface.h"
#include "Interfaces/SteamVR/SteamVRInputInterface.h"
#include "Interfaces/XInput/XInputInterface.h"

static std::vector<std::function<InputInterface*()>> InterfaceInitFuncs;
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

	for (int i = 0; i < InterfaceInitFuncs.size(); i++)
	{
		InputInterface* newinterface = InterfaceInitFuncs[i]();
		if (newinterface != nullptr)
		{
			Interfaces.push_back(newinterface);
		}
	}

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

void InputManager::RegisterInterface(std::function<InputInterface*()> CreateFunc)
{
	InterfaceInitFuncs.push_back(CreateFunc);
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

InputKeyboard * InputManager::GetKeyboard(int index, int interfaceindex)
{
	if (interfaceindex != -1)
	{
		return Interfaces[interfaceindex]->GetKeyboard(index);
	}
	int Accum = 0;
	for (int i = 0; i < Interfaces.size(); i++)
	{
		if (Interfaces[i]->GetNumOfKeyBoards() == 0)
		{
			continue;
		}
		if (Accum + Interfaces[i]->GetNumOfKeyBoards() >= index)
		{
			return Interfaces[i]->GetKeyboard(index - Accum);
		}
		Accum += Interfaces[i]->GetNumOfKeyBoards();
	}
	return nullptr;
}

InputMouse* InputManager::GetMouse(int index, int interfaceindex /*= -1*/)
{
	if (interfaceindex != -1)
	{
		return Interfaces[interfaceindex]->GetMouse(index);
	}
	int Accum = 0;
	for (int i = 0; i < Interfaces.size(); i++)
	{
		if (Interfaces[i]->GetNumOfMice() == 0)
		{
			continue;
		}
		if (Accum + Interfaces[i]->GetNumOfMice() >= index)
		{
			return Interfaces[i]->GetMouse(index - Accum);
		}
		Accum += Interfaces[i]->GetNumOfMice();
	}
	return nullptr;
}
