#include "stdafx.h"
#include "UIDropDown.h"


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

void UIDropDown::MouseClick(int x, int y)
{
	UIListBox::MouseClick(x, y);
}

void UIDropDown::SetText(std::string text)
{}

void UIDropDown::UpdateScaled()
{
	UIListBox::UpdateScaled();

}
