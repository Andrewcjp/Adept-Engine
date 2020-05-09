
#include "UIDropDown.h"
#include "UI/UIManager.h"

UIDropDown::UIDropDown(int w, int h, int x, int y) :UIListBox(w, h, x, y)
{
	Priority = 1000;
	ResizeView(w, h, x, y);
	frameCreated = RHI::GetFrameCount();
}

UIDropDown::~UIDropDown()
{}

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
