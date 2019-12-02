#include "UIButton.h"
#include "../BasicWidgets/UILabel.h"
#include "UI/UIManager.h"
UIButton::UIButton(int w, int h, int x, int y) : UIBox(w, h, x, y)
{
	Init();
	Label = new UILabel(Labelstring, 1, 0, x, y);
	Label->SetOwner(GetOwningContext());
	Colour = NormalColour;
	Priority = 2;//buttons need to draw on top of panels
	Label->Priority = Priority;
	AddChild(Label);
}
UIButton::~UIButton()
{}
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
		Colour = Hovercolour;
	}
	else
	{
		Colour = IsActiveSelect ? SelectedColour : NormalColour;
		if (WasSelected)
		{
			UIManager::UpdateBatches();
			WasSelected = false;
		}
	}
}

bool UIButton::MouseClick(int x, int y)
{
	if (!GetEnabled())
	{
		return false;
	}
	if (Rect.Contains(x, y))
	{
		if (Target)
		{
			Target();
			return true;
		}
	}
	return false;
}

void UIButton::ResizeView(int w, int h, int x, int y)
{
	UIBox::ResizeView(w, h, x, y);
	if (Label != nullptr)
	{
		Label->ResizeView(w, h / 2, x, y);
	}

	Rect = CollisionRect(w, h, x, y);
}
void UIButton::UpdateScaled()
{
	UIBox::UpdateScaled();
}

void UIButton::SetText(std::string  t)
{
	Label->SetText(t);
	Labelstring = t;
}

void UIButton::SetSelected(bool t)
{
	IsActiveSelect = t;
	Colour = IsActiveSelect ? SelectedColour : NormalColour;
	UIManager::UpdateBatches();
}

void UIButton::OnOwnerSet(UIWidgetContext * wc)
{
	Label->SetOwner(wc);
}
