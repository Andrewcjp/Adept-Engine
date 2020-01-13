#pragma once
#include "../BasicWidgets/UIBox.h"

class UIPanel :
	public UIBox
{
public:
	UIPanel(int w, int h, int x, int y);
	~UIPanel();
	void AddSubWidget(UIWidget* w);
	float ScaledHeight = 0;
	void SetTitle(std::string m);

	void UpdateScaled() override;

private:
	std::vector<UIWidget*> SubWidgets;
	class UILabel* title;
	int TextHeight = 30;
	bool UseTitle = false;
};

