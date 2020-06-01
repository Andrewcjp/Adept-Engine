#pragma once

class InputController;
class InputInterface;
class InputKeyboard;
class InputMouse;
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
	InputKeyboard* GetKeyboard(int index = 0, int iinterface = -1);
	InputMouse* GetMouse(int index = 0 , int iinterface = -1);
	InputInterface* GetActiveVRInterface();
	static void RegisterInterface(std::function<InputInterface*()> CreateFunc);
	static InputManager* Get();
private:
	std::vector<std::function<InputInterface*()>> InterfaceInitFuncs;
	std::vector<InputInterface*> Interfaces;
	InputInterface* VrInterface = nullptr;
	static InputManager* Instance;
};

template<class T, class B>
class ClassRegister
{
public:
	ClassRegister()
	{}
	ClassRegister(std::function<T*()> func)
	{
		B::RegisterInterface(func);
	}
};
