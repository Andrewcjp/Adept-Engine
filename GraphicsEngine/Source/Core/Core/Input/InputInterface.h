#pragma once

class InputController;
//this handles communication between to an input interface like OpenVR or Xinput. 
class InputInterface
{
public:
	InputInterface();
	virtual ~InputInterface();
	virtual void Tick();
	virtual InputController* GetController(int Index);
	virtual int GetNumOfControllers()const;
	//the SteamVR system is handled here and the HMD pos is updated as a special input device.
	virtual bool HasHMD()const;
	virtual void ShutDown();
protected:
	//std::vector<InputController*> Controllers;
};

