
#include "UIPopoutbox.h"
#include "UILabel.h"
#include "UI/UIManager.h"


UIPopoutbox::UIPopoutbox(int w, int h, int x, int y) :UIBox(w, h, x, y)
{
	Priority = 10;
	OkayButton = new UIButton(50, 20, x, y + (h - 20));
	OkayButton->SetText("Okay");
	OkayButton->BindTarget(std::bind(&UIPopoutbox::CloseBox, this));
	BodyLabel = new UILabel("test messahe", w, h - 20, x, y);
	ResizeView(w, h, x, y);
}

UIPopoutbox::~UIPopoutbox()
{}

void UIPopoutbox::Render()
{
	UIBox::Render();
	OkayButton->Render();
	BodyLabel->Render();
}

void UIPopoutbox::ResizeView(int w, int h, int x, int y)
{
	UIBox::ResizeView(w, h, x, y);
	OkayButton->ResizeView(50, 20, x + (w - 50), y);
	BodyLabel->ResizeView(w, h - 20, x, y);
}

void UIPopoutbox::MouseMove(int x, int y)
{
	OkayButton->MouseMove(x, y);
}

bool UIPopoutbox::MouseClick(int x, int y)
{
	return OkayButton->MouseClick(x, y);
}

void UIPopoutbox::SetText(std::string text)
{
	BodyLabel->SetText(text);
}

void UIPopoutbox::CloseBox()
{
	OkayButton->SetEnabled(false);
	SetEnabled(false);
	UIManager::UpdateBatches();
}
