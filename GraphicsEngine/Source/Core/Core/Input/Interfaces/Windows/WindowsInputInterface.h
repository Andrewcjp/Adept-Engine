#pragma once
#include "Core/Input/InputInterface.h"

class WindowsMouse;
class WindowsKeyboard;
class InputMouse;
class InputKeyboard;
class InputController;
class WindowsInputInterface : public InputInterface
{
public:
	WindowsInputInterface();
	virtual ~WindowsInputInterface();

	virtual void Tick() override;
	virtual int GetNumOfControllers() const override;
	virtual InputController* GetController(int Index) override;
	virtual int GetNumOfKeyBoards() const override;
	virtual InputKeyboard* GetKeyboard(int Index) override;
	virtual int GetNumOfMice() const override;
	virtual InputMouse* GetMouse(int Index) override;
	virtual bool HasHMD() const override { return false; };
	virtual void ShutDown() override;
	void ProcessMouseMSG(float wheelValue); 
	static WindowsInputInterface* Get() { return Instance; }
protected:
	static WindowsInputInterface* Instance;
	std::vector<WindowsMouse*> Mice;
	std::vector<WindowsKeyboard*> Keyboards;
};

