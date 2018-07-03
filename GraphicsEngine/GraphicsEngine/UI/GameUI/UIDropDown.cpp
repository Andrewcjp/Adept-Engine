#include "stdafx.h"
#include "UIDropDown.h"
#include "UI/UIManager.h"

UIDropDown::UIDropDown(int w, int h, int x, int y) :UIListBox(w, h, x, y)
{
	Priority = 10;
	ResizeView(w, h, x, y);
}

UIDropDown::~UIDropDown()
{}

void UIDropDown::Render()
{
	UIListBox::Render();
}

void UIDropDown::ResizeView(int w, int h, int x, int y)
{
	UIListBox::ResizeView(w, h, x, y);
}

void UIDropDown::MouseMove(int x, int y)
{
	UIListBox::MouseMove(x, y);
}

bool UIDropDown::MouseClick(int x, int y)
{
	bool Return = UIListBox::MouseClick(x, y);
	if (!Return)
	{
		UIManager::CloseDropDown();
	}
	return Return;
}

void UIDropDown::SetText(std::string text)
{}

void UIDropDown::UpdateScaled()
{
	UIListBox::UpdateScaled();

}
