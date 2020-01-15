
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

Inspector::InspectorPropery Inspector::CreateProperty(std::string name, EditValueType::Type type, void * Valueptr, bool EditorEffect)
{
	Inspector::InspectorPropery NameProp;
	NameProp.name = name;
	NameProp.type = type;
	NameProp.ValuePtr = Valueptr;
	NameProp.ChangesEditor = EditorEffect;
	return NameProp;
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
	UIManager::instance->CreateDropDown(ops, GetTransfrom()->GetSizeRootSpace().x, 200, GetTransfrom()->GetPositionForWidgetRootSpace().x, GetTransfrom()->GetPositionForWidgetRootSpace().y, std::bind(&Inspector::AddComponentCallback, _1));
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
	TabPanelArea->AlignGap = 40;
	TabPanelArea->TopAlignGap = 20;
	TabPanelArea->EdgeShrink = 10;
	if (TabPanelArea->Children.size() != 0)
	{
		for (int i = 0; i < TabPanelArea->Children.size(); i++)
		{
			TabPanelArea->RemoveChild(TabPanelArea->Children[i]);
		}
	}
	if (target != nullptr)
	{
		std::vector<InspectorProperyGroup> Fields = target->GetInspectorFields();
		for (int i = 0; i < Fields.size(); i++)
		{
			UIPanel* Panel = new UIPanel(0, 0, 0, 0);
			Panel->AlignGap = 10;
			Panel->TopAlignGap = 0;
			Panel->EdgeShrink = 2;
			for (int j = 0; j < Fields[i].SubProps.size(); j++)
			{
				UIWidget* newwidget = nullptr;
				switch (Fields[i].SubProps[j].type)
				{
				case EditValueType::Int:
				case EditValueType::Float:
				case EditValueType::String:
				case EditValueType::Vector:
				case EditValueType::Label:
				case EditValueType::Bool:
					newwidget = new UIEditField(&Fields[i].SubProps[j]);
					break;
				default:
					break;
				}
				if (newwidget != nullptr)
				{
					newwidget->AligmentStruct.SizeMax = 30;
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