#include "UITab.h"
#include "UIButton.h"
#include "UI/Core/UIWidget.h"
#include "UI/CompoundWidgets/UIPanel.h"
#include "UIWindow.h"
#include "UI/Core/Layout.h"

UITab::UITab()
{
	Button = new UIButton(0, 0, 0, 0);
	TabPanelArea = new UIPanel(0, 0, 0, 0);
	AddChild(TabPanelArea);
	AddChild(Button);
	TabPanelArea->BackgoundColour = glm::vec3(1, 1, 1);
	GetTransfrom()->SetStretchMode(EAxisStretch::ALL);
	TabPanelArea->AlignGap = 5;
	TabPanelArea->TopAlignGap = 20;
	TabPanelArea->EdgeShrink = 0;
}

void UITab::SetName(std::string Name)
{
	//TabPanelArea->SetTitle(Name);
	Button->SetText(Name);
	name = Name;
}

void UITab::Setup()
{
	UpdateScaled();


	Button->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);

	Button->BindTarget([this] { SelectCallback(TabIndex); });

	TabPanelArea->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	TabPanelArea->SetRootSpaceSize(GetTransfrom()->GetSizeRootSpace().x, GetTransfrom()->GetSizeRootSpace().y - ButtonHeight, 0, ButtonHeight);
	if (name.length() == 0)
	{
		SetName("Tab " + std::to_string(TabIndex));
	}
}

UITab::~UITab()
{}

void UITab::SetShowState(bool show)
{
	TabPanelArea->SetEnabled(show);
}

void UITab::UpdateScaled()
{
	const int TabWidth = 100;
	int CurrenetButtonHeight = ButtonHeight;
	Button->SetRootSpaceSize(TabWidth, ButtonHeight, TabIndex*TabWidth, 0);
	if (Parent != nullptr && Parent->AttachedTabs.size() == 1 && !AlwaysShowTabButton)
	{
		Button->SetEnabled(false);
		CurrenetButtonHeight = 0;
	}
	TabPanelArea->SetRootSpaceSize(GetTransfrom()->GetSizeRootSpace().x, GetTransfrom()->GetSizeRootSpace().y - CurrenetButtonHeight, 0, CurrenetButtonHeight);
	if (ListItems)
	{
		glm::ivec2 Space = TabPanelArea->GetTransfrom()->GetSizeRootSpace();
		UIUtils::ArrangeHorizontal(Space.x - TabPanelArea->EdgeShrink * 2, Space.y, TabPanelArea->EdgeShrink, TabPanelArea->TopAlignGap, TabPanelArea->Children, 0, TabPanelArea->AlignGap);
	}
	UIWidget::UpdateScaled();
}
