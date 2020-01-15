
#include "UIPanel.h"
#include "../Core/Layout.h"
#include "../BasicWidgets/UILabel.h"
UIPanel::UIPanel(int w, int  h, int  x, int y) :UIBox(w, h, x, y)
{
	title = new UILabel("title", w, 30, x, y);
	TextHeight = 50;
	AligmentStruct.SizeMax = 0.05f;
	title->TextScale = 0.3f;
	title->SetRootSpaceScaled(w, TextHeight, 0, TextHeight);
	title->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	BatchMode = EWidgetBatchMode::On;
	IgnoreboundsCheck = true;
}


UIPanel::~UIPanel()
{}

void UIPanel::AddSubWidget(UIWidget * w)
{
	SubWidgets.push_back(w);
	w->IgnoreboundsCheck = true;
	w->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	AddChild(w);
	AligmentStruct.SizeMax = TextHeight + TopAlignGap;
	for (int i = 0; i < SubWidgets.size(); i++)
	{
		AligmentStruct.SizeMax += SubWidgets[i]->AligmentStruct.SizeMax + AlignGap;
	}
}

void UIPanel::SetTitle(std::string m)
{
	title->SetText(m);
	if (!UseTitle)
	{
		AddChild(title);
		UseTitle = true;
	}
}

void UIPanel::UpdateScaled()
{
	UIBox::UpdateScaled();
	if (!UseTitle)
	{
		TextHeight = 0;
	}
	else
	{
		TextHeight = 30;
		title->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
		title->SetRootSpaceSize(GetTransfrom()->GetSizeRootSpace().x, TextHeight, 0, TextHeight / 3);
	}
	UIUtils::ArrangeHorizontal(GetTransfrom()->GetSizeRootSpace().x - EdgeShrink * 2, GetTransfrom()->GetSizeRootSpace().y - TextHeight, EdgeShrink, TextHeight + TopAlignGap, SubWidgets, -1, AlignGap);
}
