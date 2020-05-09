#pragma once
#include "../Core/UIWidget.h"
#include "UIButton.h"

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
	void UpdateScaled()override;
	void RemoveAll();
	void Select(int i);
	void AddItem(std::string  name);
	void SetTitle(std::string name);
	std::function<void(int t)> SelectionChanged;
	int ItemHeight = 20;
	int ItemSize = 20;
private:
	UIWidget* MaskedPanel = nullptr;
	std::vector<UIButton*> items;
	int SelectedCurrent = 0;
	int CurrentCount = 0;
	int TitleHeight = 20;
	int Spacing = ItemHeight + 6;
	int Edgespace = 5;
	float CurrentScroll = 0;
	bool ShowScrollBar = false;
	UIBox* Background = nullptr;
	UILabel* TitleLabel = nullptr;
	float Scorll = 0;
protected:
	void ProcessUIInputEvent(UIInputEvent& e) override;

};

