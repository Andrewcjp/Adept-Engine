
#include "Inspector.h"
#include "IEdtiorInspectable.h"

#include "UI/CompoundWidgets/UIEditField.h"
#include "UI/BasicWidgets/UIBox.h"
#include "UI/UIManager.h"
#include "UI/CompoundWidgets/UIButton.h"
#include "UI/Core/Layout.h"
#include "UI/CompoundWidgets/UIPanel.h"
#include "Core/Components/ComponentRegistry.h"
#include "Core/GameObject.h"
#include "Core/Components/Component.h"
#include <vector>
#include "UI/EditorUI/UIPropertyField.h"
#if WITH_EDITOR
Inspector* Inspector::Instance = nullptr;
Inspector::Inspector(int w, int h, int x, int y)
	:UITab()
{
	CreateEditor();
	ensure(Instance == nullptr);
	//AddChild(Backgroundbox);
	Instance = this;
	SetName("Inspector");
}

Inspector::~Inspector()
{
	Instance = nullptr;
}

void Inspector::SetSelectedObject(IEdtiorInspectable * newTarget)
{
	if (target != newTarget)
	{
		target = newTarget;
		CreateEditor();
	}
}
void Inspector::Refresh()
{
	CreateEditor();
}

InspectorProperyGroup Inspector::CreatePropertyGroup(std::string name)
{
	InspectorProperyGroup NameProp;
	NameProp.name = name;
	return NameProp;
}

void Inspector::MouseMove(int x, int y)
{
	UITab::MouseMove(x, y);
}

bool Inspector::MouseClick(int x, int y)
{
	UITab::MouseClick(x, y);
	return false;
}
void Inspector::MouseClickUp(int x, int y)
{
	UITab::MouseClickUp(x, y);
}

void Inspector::AddComponent()
{
	//__debugbreak();
	std::vector<std::string> ops;
	for (int i = 0; i < ComponentRegistry::GetInstance()->GetCount(); i++)
	{
		ops.push_back(ComponentRegistry::GetInstance()->GetNameById(i));
	}

	using std::placeholders::_1;
	UIManager::instance->CreateDropDown(ops, button->GetTransfrom()->GetSizeRootSpace().x, 200, button->GetTransfrom()->GetPositionForWidgetRootSpace().x, button->GetTransfrom()->GetPositionForWidgetRootSpace().y, std::bind(&Inspector::AddComponentCallback, _1));
}

void Inspector::AddComponentCallback(int i)
{
	//__debugbreak();
	//close dropdown
	if (Instance != nullptr)
	{
		if (Instance->target != nullptr)
		{
			GameObject* t = (GameObject*)Instance->target;
			if (t != nullptr)
			{
				Component* comp = ComponentRegistry::CreateBaseComponent(ComponentRegistry::BaseComponentTypes(i));
				if (comp != nullptr)
				{
					t->AttachComponent(comp);
					comp->SceneInitComponent();
					Instance->CreateEditor();
				}
			}
		}
	}
	UIManager::CloseDropDown();
}

void Inspector::UpdateScaled()
{
	glm::ivec2 Space = TabPanelArea->GetTransfrom()->GetSizeRootSpace();
	UIUtils::ArrangeHorizontal(Space.x - TabPanelArea->EdgeShrink * 2, Space.y, TabPanelArea->EdgeShrink, TabPanelArea->TopAlignGap, TabPanelArea->Children, 0, TabPanelArea->AlignGap);
	UITab::UpdateScaled();
}

void Inspector::CreateEditor()
{
	TabPanelArea->AlignGap = 10;
	TabPanelArea->TopAlignGap = 5;
	TabPanelArea->EdgeShrink = 10;
	TabPanelArea->RemoveAllChildren();
	if (target != nullptr)
	{
		std::vector<InspectorProperyGroup> Fields = target->GetInspectorFields();
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
		button = new UIButton(mwidth, 30, 0, 0);
		button->SetRootSpaceSize(250, 50, 0, 0);
		button->SetText("Add Component");
		button->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
		button->BindTarget(std::bind(&Inspector::AddComponent, this));
		button->AligmentStruct.SizeMax = 50;
		TabPanelArea->AddChild(button);
	}
}
#endif