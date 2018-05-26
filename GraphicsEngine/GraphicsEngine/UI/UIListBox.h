#pragma once
#include "UIWidget.h"
#include "UIButton.h"
#include <vector>
class UIBox;
class UIListBox :
	public UIWidget
{
public:
	UIListBox(int w, int h, int x, int y);
	virtual ~UIListBox();

	// Inherited via UIWidget
	virtual void Render() override;
	void MouseMove(int x, int y)override;
	bool MouseClick(int x, int y)override;
	void MouseClickUp(int x, int y)override;
	virtual void ResizeView(int w, int h, int x = 0, int y = 0) override;
	void UpdateScaled()override;
	void RemoveAll();
	void Select(int i);
	void AddItem(std::string  name);
	void SetTitle(std::string name);

	std::function<void(int t)> SelectionChanged;
private:
	std::vector<UIButton*> items;
	int SelectedCurrent = 0;
	int CurrentCount = 0;
	int TitleHeight = 20;
	int ItemHeight = 20;
	int Spacing = ItemHeight + 6;
	int Edgespace = 5;
	float CurrentScroll = 0;
	bool ShowScrollBar = false;
	UIBox* Background;
	UILabel* TitleLabel;
};

