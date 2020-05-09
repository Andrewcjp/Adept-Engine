#pragma once
#include "UI/CompoundWidgets/UIListBox.h"
class UIDropDown :public UIListBox
{
public:
	UIDropDown(int w, int h, int x, int y);
	virtual ~UIDropDown();
	bool MouseClick(int x, int y) override;
	void SetText(std::string text);
	void UpdateScaled() override;
	int frameCreated = 0;
};

