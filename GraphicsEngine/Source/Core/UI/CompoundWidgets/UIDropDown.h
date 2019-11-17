#pragma once
#include "UI/CompoundWidgets/UIListBox.h"
class UIDropDown :public UIListBox
{
public:
	UIDropDown(int w, int h, int x, int y);
	virtual ~UIDropDown();
	void Render() override;
	void ResizeView(int w, int h, int x, int y) override;
	void MouseMove(int x, int y) override;
	bool MouseClick(int x, int y) override;
	void SetText(std::string text);
	void UpdateScaled() override;
	int frameCreated = 0;
};

