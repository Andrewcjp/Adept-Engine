#include "stdafx.h"
#include "UIPanel.h"
#include "Layout.h"
#include "UILabel.h"
UIPanel::UIPanel(int w, int  h, int  x, int y) :UIBox(w, h, x, y)
{
	title = new UILabel("title", w, 30, x, y);
	TextHeight = 15;
	AligmentStruct.SizeMax = 0.05f;
	title->TextScale = 0.3f;

}


UIPanel::~UIPanel()
{
}

void UIPanel::AddSubWidget(UIWidget * w)
{
	AligmentStruct.SizeMax += w->AligmentStruct.SizeMax;
	SubWidgets.push_back(w);
}

void UIPanel::ResizeView(int w, int h, int x, int y)
{
	UIBox::ResizeView(w, h, x, y);
	title->ResizeView(w, TextHeight, x + (w / 4), y + h - TextHeight * 2);
	UIUtils::ArrangeHorizontal(w, h - TextHeight, x, y - TextHeight, SubWidgets);
}

void UIPanel::MouseMove(int x, int y)
{
	for (int i = 0; i < SubWidgets.size(); i++)
	{
		SubWidgets[i]->MouseMove(x, y);
	}
}

void UIPanel::MouseClick(int x, int y)
{
	for (int i = 0; i < SubWidgets.size(); i++)
	{
		SubWidgets[i]->MouseClick(x, y);
	}
}

void UIPanel::MouseClickUp(int x, int y)
{
	for (int i = 0; i < SubWidgets.size(); i++)
	{
		SubWidgets[i]->MouseClickUp(x, y);
	}
}

void UIPanel::Render()
{
	UIBox::Render();
	for (int i = 0; i < SubWidgets.size(); i++)
	{
		SubWidgets[i]->Render();
	}
	title->Render();
}

void UIPanel::SetTitle(std::string m)
{
	title->SetText(m);
}
