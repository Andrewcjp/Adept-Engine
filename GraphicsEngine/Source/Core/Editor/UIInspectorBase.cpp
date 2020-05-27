
#include "UIInspectorBase.h"
#include "IEdtiorInspectable.h"

#include "UI/CompoundWidgets/UIEditField.h"
#include "UI/BasicWidgets/UIBox.h"
#include "UI/UIManager.h"
#include "UI/CompoundWidgets/UIButton.h"
#include "UI/Core/Layout.h"
#include "UI/CompoundWidgets/UIPanel.h"
#include "Core/GameObject.h"
#include "Core/Components/Component.h"
#include <vector>
#include "UI/EditorUI/UIPropertyField.h"
#include "Core/Reflection/ObjectLibrary.h"
#if WITH_EDITOR

UIInspectorBase::UIInspectorBase(int w, int h, int x, int y)
	:UITab()
{
	SetName("Inspector");
	Refresh();
}

UIInspectorBase::~UIInspectorBase()
{

}

void UIInspectorBase::Refresh()
{
	CreateEditor();
}

InspectorProperyGroup UIInspectorBase::CreatePropertyGroup(std::string name)
{
	InspectorProperyGroup NameProp;
	NameProp.name = name;
	return NameProp;
}

void UIInspectorBase::MouseMove(int x, int y)
{
	UITab::MouseMove(x, y);
}

bool UIInspectorBase::MouseClick(int x, int y)
{
	UITab::MouseClick(x, y);
	return false;
}
void UIInspectorBase::MouseClickUp(int x, int y)
{
	UITab::MouseClickUp(x, y);
}



void UIInspectorBase::UpdateScaled()
{
	glm::ivec2 Space = TabPanelArea->GetTransfrom()->GetSizeRootSpace();
	UIUtils::ArrangeHorizontal(Space.x - TabPanelArea->EdgeShrink * 2, Space.y, TabPanelArea->EdgeShrink, TabPanelArea->TopAlignGap, TabPanelArea->Children, 0, TabPanelArea->AlignGap);
	UITab::UpdateScaled();
}

void UIInspectorBase::CreateEditor()
{
	Fields.clear();
	OnPopulateFields();
	TabPanelArea->AlignGap = 10;
	TabPanelArea->TopAlignGap = 5;
	TabPanelArea->EdgeShrink = 10;
	TabPanelArea->RemoveAllChildren();


	for (int i = 0; i < Fields.size(); i++)
	{
		UIPanel* Panel = new UIPanel(0, 0, 0, 0);
		Panel->AlignGap = 5;
		Panel->TopAlignGap = 0;
		Panel->EdgeShrink = 2;
		Panel->TextHeight = 30;
		for (int j = 0; j < Fields[i].Nodes.size(); j++)
		{
			UIPropertyField* newwidget = nullptr;
			switch (Fields[i].Nodes[j]->m_Type)
			{
			case MemberValueType::Int:
			case MemberValueType::Float:
			case MemberValueType::String:
			case MemberValueType::Vector2:
			case MemberValueType::Vector3:
			case MemberValueType::Bool:
			case MemberValueType::AssetPtr:
				//newwidget = new UIEditField(Fields[i].Nodes[j]);
				newwidget = new UIPropertyField();
				newwidget->SetTarget(Fields[i].Nodes[j]);
				break;
			default:
				break;
			}
			if (newwidget != nullptr)
			{
				newwidget->AligmentStruct.SizeMax = 20;
				Panel->AddSubWidget(newwidget);
			}
		}
		Panel->SetTitle(Fields[i].name);
		Panel->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
		//Panel->AligmentStruct.SizeMax = 40;
		TabPanelArea->AddChild(Panel);
	}
	OnInspectorFinishCreate(TabPanelArea);
	/*button = new UIButton(mwidth, 30, 0, 0);
	button->SetRootSpaceSize(250, 50, 0, 0);
	button->SetText("Add Component");
	button->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	button->BindTarget(std::bind(&InspectorBase::AddComponent, this));
	button->AligmentStruct.SizeMax = 50;
	TabPanelArea->AddChild(button);*/

}
#endif