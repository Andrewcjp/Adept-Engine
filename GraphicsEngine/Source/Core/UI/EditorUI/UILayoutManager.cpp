#include "Stdafx.h"
#include "UILayoutManager.h"
#include "UI/Core/UIWidgetContext.h"
#include "UI/UIManager.h"
#include "UI/CompoundWidgets/UIWindow.h"


UILayoutManager::UILayoutManager()
{
	Context = UIManager::GetDefaultContext();
}

UILayoutManager::~UILayoutManager()
{}


void UILayoutManager::UpdateSlotSizes()
{
	const int TopSize = 50;
	const int BottomSize = 200;
	const int SizeSize = 300;


	Slots[Left].Size = glm::vec2(SizeSize, Context->RootSpaceViewport.Max.y - BottomSize - TopSize);
	Slots[Left].Pos = glm::vec2(0, BottomSize);

	Slots[Right].Size = glm::vec2(SizeSize, Context->RootSpaceViewport.Max.y - BottomSize - TopSize);
	Slots[Right].Pos = glm::vec2(Context->RootSpaceViewport.Max.x - Slots[Right].Size.x, BottomSize);

	Slots[Bottom].Size = glm::vec2(Context->RootSpaceViewport.Max.x, BottomSize);

	Slots[Top].Size = glm::vec2(Context->RootSpaceViewport.Max.x, TopSize);
	Slots[Top].Pos = glm::vec2(0, Context->RootSpaceViewport.Max.y - TopSize);
}

void UILayoutManager::Update()
{
	UpdateSlotSizes();
	Slots[Centre].Pos.x = Slots[Left].GetSize().x;
	Slots[Centre].Pos.y = Slots[Bottom].GetSize().y;
	Slots[Centre].Size.x = Context->RootSpaceViewport.Max.x - Slots[Right].GetSize().x - Slots[Left].GetSize().x;
	Slots[Centre].Size.y = Context->RootSpaceViewport.Max.y - Slots[Top].GetSize().y - Slots[Bottom].GetSize().y;
	PushToSlots();
}

void UILayoutManager::PushToSlots()
{
	for (int i = 0; i < Limit; i++)
	{
		if (Slots[i].Widget != nullptr)
		{
			Slots[i].Widget->SetRootSpaceSize(Slots[i].Size.x, Slots[i].Size.y, Slots[i].Pos.x, Slots[i].Pos.y);
		}
	}
}

void UILayoutManager::SetWidget(EDockedSlotAnchour slot, UIWidget * widget)
{
	Slots[slot].Widget = widget;
	UIWindow* win = dynamic_cast<UIWindow*>(widget);
	if (win != nullptr)
	{
		win->SetIsFloating(false);
	}
}

glm::ivec2 UILayoutManager::DockedSlot::GetSize()
{
	if (Widget != nullptr)
	{
		return Size;
	}
	return glm::ivec2();
}
