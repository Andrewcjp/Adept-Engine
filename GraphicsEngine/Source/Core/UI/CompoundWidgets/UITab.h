#pragma once
#include "UI/Core/UIWidget.h"

class UIButton;
class UIPanel;
class UIWindow;
class UITab : public UIWidget
{
public:
	UITab();
	void SetName(std::string Name);
	void Setup();
	~UITab();
	int TabIndex = 0;
	std::function<void(int)> SelectCallback;
	UIButton* Button = nullptr;
	void SetShowState(bool show);
	UIPanel* TabPanelArea = nullptr;
	std::string name = "";

	void UpdateScaled() override;
	int ButtonHeight = 20;
	UIWindow* Parent = nullptr;
	bool ListItems = false;
	bool AlwaysShowTabButton = false;
};

