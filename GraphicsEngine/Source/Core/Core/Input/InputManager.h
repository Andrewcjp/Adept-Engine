#pragma once

class InputController;
class InputInterface;
//Owns all InputControllers 
class InputManager
{
public:
	InputManager();
	~InputManager();
	void InitInterfaces();
	void Tick();
	void EnumConnectedDevices();
	//if interface is -1 (default) the index starts at the first interface with a controller.
	InputController* GetController(int index, int iinterface = -1);
	InputInterface* GetActiveVRInterface();
private:
	std::vector<InputInterface*> Interfaces;
	InputInterface* VrInterface = nullptr;
};

