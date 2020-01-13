#include "UITab.h"
#include "UIButton.h"
#include "UI/Core/UIWidget.h"
#include "UI/CompoundWidgets/UIPanel.h"
#include "UIWindow.h"

UITab::UITab()
{
	Button = new UIButton(0, 0, 0, 0);
	TabPanelArea = new UIPanel(0, 0, 0, 0);
	AddChild(TabPanelArea);
	AddChild(Button);
	TabPanelArea->BackgoundColour = glm::vec3(1, 1, 1);
	GetTransfrom()->SetStretchMode(EAxisStretch::ALL);
}

void UITab::SetName(std::string Name)
{
	TabPanelArea->SetTitle(Name);
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
	if (Parent!= nullptr && Parent->AttachedTabs.size() == 1)
	{
		Button->SetEnabled(false);
		CurrenetButtonHeight = 0;
	}
	TabPanelArea->SetRootSpaceSize(GetTransfrom()->GetSizeRootSpace().x, GetTransfrom()->GetSizeRootSpace().y - CurrenetButtonHeight, 0, CurrenetButtonHeight);
	UIWidget::UpdateScaled();
}
