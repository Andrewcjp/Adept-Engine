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
	int AlignGap = 0;
	int TopAlignGap = 0;
	int EdgeShrink = 0;
	int TextHeight = 30;
private:
	std::vector<UIWidget*> SubWidgets;
	class UILabel* title;
	bool UseTitle = false;
};

