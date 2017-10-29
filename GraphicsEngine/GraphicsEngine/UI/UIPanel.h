#pragma once
#include "UIBox.h"
#include <vector>
class UIPanel :
	public UIBox
{
public:
	UIPanel(int w, int h, int x, int y);
	~UIPanel();
	void AddSubWidget(UIWidget* w);
	virtual void ResizeView(int w, int h, int x, int y) override;
	void MouseMove(int x, int y) override;
	void MouseClick(int x, int y) override;
	void MouseClickUp(int x, int y) override;
	virtual void Render() override;
	float ScaledHeight = 0;
	void SetTitle(std::string m);
private:
	std::vector<UIWidget*> SubWidgets;
	class UILabel* title;
	int TextHeight = 10;
};

