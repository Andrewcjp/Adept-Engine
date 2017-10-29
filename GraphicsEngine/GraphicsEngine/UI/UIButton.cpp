#include "UIButton.h"
#include "UILabel.h"
#include "UIManager.h"
UIButton::UIButton(int w, int h, int x, int y) : UIBox(w, h, x, y)
{
	Init();
	Label = new UILabel(MText, 1, 0, x, y);
	Colour = glm::vec3(0.5f);
}
UIButton::~UIButton()
{
}
void UIButton::Render()
{
	UIBox::Render();
	Label->Render();
}
UILabel* UIButton::GetLabel()
{
	return Label;
}
void UIButton::MouseMove(int x, int y)
{
	if (Rect.Contains(x, y))
	{
		WasSelected = true;
		UIManager::UpdateBatches();
		Colour = glm::vec3(1, 0, 0);
	}
	else
	{		
		Colour = glm::vec3(0.5f);
		if (WasSelected)
		{
			UIManager::UpdateBatches();
			WasSelected = false;
		}
	}
}

void UIButton::MouseClick(int x, int y)
{
	if (Rect.Contains(x, y))
	{
		Target();
	}
}

void UIButton::ResizeView(int w, int h, int x, int y)
{
	UIBox::ResizeView(w, h, x, y);
	if (Label != nullptr)
	{
		Label->ResizeView(w, h/2, x, y);
	}

	Rect = CollisionRect(w, h, x, y);
}
void UIButton::UpdateScaled()
{
	UIBox::UpdateScaled();
	//Rect = CollisionRect(mwidth, mheight, X, Y);
}

void UIButton::SetText(std::string  t)
{
	Label->SetText(t);
}
