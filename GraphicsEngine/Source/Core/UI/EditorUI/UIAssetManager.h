#pragma once
#include "UI/Core/UIBox.h"
class UIListBox;
class UIAssetManager : public UIBox
{
public:
	UIAssetManager();
	~UIAssetManager();
	void ResizeView(int w, int h, int x, int y) override;
	void MouseMove(int x, int y) override;
	bool MouseClick(int x, int y) override;
private:
	UIListBox* leftpanel;
	float LeftSideWidth = 0.15f;
};

