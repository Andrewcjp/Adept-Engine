#pragma once
#include "UI/BasicWidgets/UIBox.h"
#include "UI/CompoundWidgets/UITab.h"
class UIListBox;
class BaseAsset;
class UIAssetManager : public UITab
{
public:
	UIAssetManager();
	~UIAssetManager();


	void UpdateScaled() override;

	void Build();

	void SelectedCallback(int i);

private:
	UIListBox* leftpanel;
	std::vector<BaseAsset*> Assets;
	float LeftSideWidth = 0.15f;
};

