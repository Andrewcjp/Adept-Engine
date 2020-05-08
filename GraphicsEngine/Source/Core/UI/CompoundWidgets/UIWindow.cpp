#include "UIWindow.h"
#include "UI/BasicWidgets/UILabel.h"
#include "UIPanel.h"
#include "Rendering/Core/Screen.h"
#include "UI/Core/UIWidgetContext.h"
#include "UITab.h"


UIWindow::UIWindow() :UIBox(0, 0, 0, 0)
{
	Title = new UILabel("Title", 0, 0, 0, 0);
	Title->IgnoreboundsCheck = true;
	WorkArea = new UIPanel(0, 0, 0, 0);
	//	WorkArea->SetTitle("jweiort");
	WorkArea->IgnoreboundsCheck = true;
	IgnoreboundsCheck = true;
	AddChild(Title);
	AddChild(WorkArea);
	UpdateSize();
	Priority = 1000;
	IsFloating = true;

}

void UIWindow::UpdateSize()
{
	int Sizeoffset = 0;
	Title->SetEnabled(IsFloating);
	if (IsFloating)
	{
		Title->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
		Title->SetRootSpaceSize(GetTransfrom()->GetSizeRootSpace().x, TabSize, 0, 0);
		Sizeoffset = TabSize + 10;
	}
	WorkArea->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	WorkArea->SetRootSpaceSize(GetTransfrom()->GetSizeRootSpace().x, GetTransfrom()->GetSizeRootSpace().y - Sizeoffset, 0, Sizeoffset);

}

UIWindow::~UIWindow()
{}

void UIWindow::UpdateScaled()
{
	UIBox::UpdateScaled();
	UIWidget::UpdateScaled();
	UpdateSize();
}

void UIWindow::MouseMove(int x, int y)
{
	UIBox::MouseMove(x, y);
	if (!Drag ||!IsFloating)
	{
		return;
	}
	glm::ivec2 currentpos = ConvertScreenToRootSpace(glm::ivec2(x, y));
	if (DidJustDrag)
	{
		LastPos = currentpos;
		DidJustDrag = false;
	}
	glm::ivec2 delta = currentpos - LastPos;
	GetTransfrom()->SetPos(GetTransfrom()->GetPos() + IntPoint(delta.x, delta.y));
	LastPos = currentpos;
}

bool UIWindow::MouseClick(int x, int y)
{
	UIWidget::MouseClick(x, y);
	if (IsFloating)
	{
		if (Title->ContainsPoint(ConvertScreenToRootSpace(glm::ivec2(x, y))))
		{
			Drag = true;
			DidJustDrag = true;
		}
	}
	return true;
}

void UIWindow::MouseClickUp(int x, int y)
{
	UIWidget::MouseClickUp(x, y);
	Drag = false;
}

void UIWindow::AddTab(UITab * Tab)
{
	Tab->TabIndex = AttachedTabs.size();
	AttachedTabs.push_back(Tab);
	WorkArea->AddChild(Tab);
	Tab->SetRootSpaceSize(WorkArea->GetTransfrom()->GetSizeRootSpace().x, WorkArea->GetTransfrom()->GetSizeRootSpace().x, 0, 0);
	Tab->Setup();
	Tab->SelectCallback = std::bind(&UIWindow::FocusTab, this, std::placeholders::_1);
	if (Tab->TabIndex > 0)
	{
		Tab->SetShowState(false);
	}
	Tab->Parent = this;
}

void UIWindow::FocusTab(int index)
{
	AttachedTabs[CurrentTab]->SetShowState(false);
	AttachedTabs[index]->SetShowState(true);
	CurrentTab = index;
}

void UIWindow::SetIsFloating(bool val)
{
	IsFloating = val;
}

void UIWindow::OnGatherBatches(UIRenderBatch* Groupbatchptr /*= nullptr*/)
{
	UIBox::OnGatherBatches(Groupbatchptr);
}
