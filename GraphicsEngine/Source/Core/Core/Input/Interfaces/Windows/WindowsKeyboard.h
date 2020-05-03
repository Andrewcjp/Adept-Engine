#pragma once
#include "Core/Input/InputKeyboard.h"
class WindowsKeyboard  :public InputKeyboard
{
public:
	WindowsKeyboard();
	~WindowsKeyboard();



protected:
	void UpdateState() override;

};

