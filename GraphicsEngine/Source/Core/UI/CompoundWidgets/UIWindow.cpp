#include "UIWindow.h"
#include "UI/BasicWidgets/UILabel.h"
#include "UIPanel.h"
#include "Rendering/Core/Screen.h"
#include "UI/Core/UIWidgetContext.h"
#include "UITab.h"
#include "UIButton.h"


UIWindow::UIWindow() :UIBox(0, 0, 0, 0)
{
	m_TitleLabel = new UILabel("Title", 0, 0, 0, 0);
	m_TitleLabel->IgnoreboundsCheck = true;
	WorkArea = new UIPanel(0, 0, 0, 0);
	//	WorkArea->SetTitle("jweiort");
	WorkArea->IgnoreboundsCheck = true;
	IgnoreboundsCheck = true;
	AddChild(m_TitleLabel);
	AddChild(WorkArea);
	UpdateSize();
	Priority = 10;
	IsFloating = true;

}

void UIWindow::UpdateSize()
{
	int Sizeoffset = 0;
	m_TitleLabel->SetEnabled(IsFloating);
	if (IsFloating)
	{
		m_TitleLabel->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
		m_TitleLabel->SetRootSpaceSize(GetTransfrom()->GetSizeRootSpace().x, TabSize, 0, 0);
		Sizeoffset = TabSize + 10;
	}
	WorkArea->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	WorkArea->SetRootSpaceSize(GetTransfrom()->GetSizeRootSpace().x, GetTransfrom()->GetSizeRootSpace().y - Sizeoffset, 0, Sizeoffset);
	if (CloseButton != nullptr)
	{
		int Size = 20;
		CloseButton->SetRootSpaceSize(Size, 25, GetTransfrom()->GetSizeRootSpace().x - Size, 0);
	}
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
	if (!Drag || !IsFloating)
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
		if (m_TitleLabel->ContainsPoint(ConvertScreenToRootSpace(glm::ivec2(x, y))))
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

void UIWindow::SetCloseable(bool state)
{
	IsCloseAble = state;
	if (!IsCloseAble)
	{
		SafeDelete(CloseButton);
		return;
	}
	if (CloseButton == nullptr)
	{
		CloseButton = new UIButton(0, 0);
		CloseButton->SetText("X");
		CloseButton->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
		CloseButton->BindTarget(std::bind(&UIWindow::CLose, this));
		AddChild(CloseButton);
	}
}

void UIWindow::SetTitle(const std::string& title)
{
	m_TitleLabel->SetText(title);
}

void UIWindow::CLose()
{
	SetEnabled(false);
}

void UIWindow::OnGatherBatches(UIRenderBatch* Groupbatchptr /*= nullptr*/)
{
	UIBox::OnGatherBatches(Groupbatchptr);
}
