#include "stdafx.h"
#include "UIPopoutbox.h"




UIPopoutbox::UIPopoutbox(int w, int h, int x, int y) :UIBox(w, h, x, y)
{
	Priority = 10;
	OkayButton = new UIButton(50, 20, x, y + (h - 20));
	OkayButton->SetText("Okay");
	OkayButton->BindTarget(std::bind(&UIPopoutbox::CloseBox, this));
	ResizeView(w, h, x, y);
}

UIPopoutbox::~UIPopoutbox()
{
}
 
void UIPopoutbox::Render()
{
	UIBox::Render();
	OkayButton->Render();
}

void UIPopoutbox::ResizeView(int w, int h, int x, int y)
{
	UIBox::ResizeView(w, h, x, y);
	OkayButton->ResizeView(50, 20, x + (w - 50), y);
}

void UIPopoutbox::MouseMove(int x, int y)
{
	OkayButton->MouseMove(x, y);
}

void UIPopoutbox::MouseClick(int x, int y)
{
	OkayButton->MouseClick(x, y);
}

void UIPopoutbox::CloseBox()
{
	OkayButton->SetEnabled(false);
	SetEnabled(false);
}
