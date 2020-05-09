#pragma once
#include "UI/BasicWidgets/UIBox.h"
#include "UI/CompoundWidgets/UITab.h"
class UIListBox;
class UIAssetManager : public UITab
{
public:
	UIAssetManager();
	~UIAssetManager();

private:
	UIListBox* leftpanel;
	float LeftSideWidth = 0.15f;
};

