#pragma once

class InputController;
class InputKeyboard;
class InputMouse;
//this handles communication between to an input interface like OpenVR or Xinput. 
class InputInterface
{
public:
	InputInterface();
	virtual ~InputInterface();
	virtual void Tick();
	virtual int GetNumOfControllers()const;
	virtual InputController* GetController(int Index);
	virtual int GetNumOfKeyBoards() const;
	virtual InputKeyboard* GetKeyboard(int Index);
	virtual int GetNumOfMice() const;
	virtual InputMouse* GetMouse(int Index);
	//the SteamVR system is handled here and the HMD pos is updated as a special input device.
	virtual bool HasHMD()const;
	virtual void ShutDown();
protected:
	//std::vector<InputController*> Controllers;
};

