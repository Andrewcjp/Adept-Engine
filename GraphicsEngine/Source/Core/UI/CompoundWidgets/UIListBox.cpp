#include "UIListBox.h"
#include "../BasicWidgets/UIBox.h"
#include "../BasicWidgets/UILabel.h"
#include "../Core/UIWidget.h"
#include "../Core/Layout.h"
#include "Core/Input/Input.h"
UIListBox::UIListBox(int w, int h, int x, int y) : UIWidget(w, h, x, y)
{

	Background = new UIBox(w, h, x, y);
	Background->Colour = glm::vec3(0.7f);
	TitleLabel = new UILabel("List Box", w, 20, x, y + mheight - 20);
	AddChild(Background);
	AddChild(TitleLabel);
	MaskedPanel = new UIWidget();
	AddChild(MaskedPanel);
	BatchMode = EWidgetBatchMode::On;
}

UIListBox::~UIListBox()
{}

void UIListBox::Render()
{
	Background->Render();
	TitleLabel->Render();
	for (int i = 0; i < items.size(); i++)
	{
		items[i]->Render();
	}
}

void UIListBox::MouseMove(int x, int y)
{
	for (int i = 0; i < items.size(); i++)
	{
		items[i]->MouseMove(x, y);
		Background->MouseMove(x, y);
	}
}

bool UIListBox::MouseClick(int x, int y)
{
	bool ReturnValue = false;
	for (int i = 0; i < items.size(); i++)
	{
		ReturnValue |= items[i]->MouseClick(x, y);
		ReturnValue |= Background->MouseClick(x, y);
	}
	return ReturnValue;
}

void UIListBox::MouseClickUp(int x, int y)
{
	for (int i = 0; i < items.size(); i++)
	{
		items[i]->MouseClickUp(x, y);
		Background->MouseClickUp(x, y);
	}
}

void UIListBox::UpdateScaled()
{
	UIWidget::UpdateScaled();
	int w = GetTransfrom()->GetSizeRootSpace().x;
	int h = GetTransfrom()->GetSizeRootSpace().y;
	Background->SetRootSpaceSize(w, h, 0, 0);

	TitleLabel->SetRootSpaceSize(w, 10, 0, mheight - 20);
	const int EdgeOffset = 40;
	const int RealHeight = h - EdgeOffset * 2;
	MaskedPanel->SetRootSpaceSize(w - Edgespace, RealHeight, Edgespace / 2, EdgeOffset);
	Scorll -= Input::Get()->GetMouseWheelAxis()*2;
	float ItemsLength = ItemSize * items.size();
	Scorll = glm::clamp(Scorll,0.0f, (float)(ItemsLength - RealHeight));
	UIUtils::ArrangeHorizontal(w - Edgespace, h - 40, Edgespace / 2, -Scorll, items, 0.05f, 0, 20);
}

void UIListBox::RemoveAll()
{
	for (int i = 0; i < items.size(); i++)
	{
		delete items[i];
	}
	CurrentCount = 0;
	items.clear();
}

void UIListBox::Select(int index)
{
	SelectedCurrent = index;
	if (SelectionChanged)
	{
		SelectionChanged(SelectedCurrent);
	}
	for (int i = 0; i < items.size(); i++)
	{
		items[i]->SetSelected(i == SelectedCurrent);
	}
}

void UIListBox::AddItem(std::string name)
{
	UIButton* button = new UIButton(mwidth - Edgespace, ItemHeight, Edgespace / 2, (Y + mheight) - (TitleHeight + (CurrentCount + 1) * Spacing));
	//button->GetLabel()->TextScale = 0.3f;
	button->SetText(name);
	button->AligmentStruct.SizeMax = ItemSize;
	button->BindTarget(std::bind(&UIListBox::Select, this, CurrentCount));
	button->BackgoundColour = Background->Colour;
	button->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	items.push_back(button);
	CurrentCount++;
	MaskedPanel->AddChild(button);
}

void UIListBox::SetTitle(std::string name)
{
	TitleLabel->SetText(name);
}

