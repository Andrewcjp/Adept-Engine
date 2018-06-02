#include "stdafx.h"
#include "Inspector.h"
#include "IInspectable.h"
#include <vector>
#include "../UI/UIEditField.h"
#include "../UI/UIBox.h"
#include "../UI/UIManager.h"
#include "../UI/UIButton.h"
#include "../UI/Layout.h"
#include "../UI/UIPanel.h"
#include "../Core/Components/CompoenentRegistry.h"
#include "../Core/GameObject.h"
#include "../Core/Components/Component.h"
Inspector* Inspector::Instance = nullptr;
Inspector::Inspector(int w, int h, int x, int y)
	:UIWidget(w, h, x, y)
{
	Backgroundbox = new UIBox(w, h, x, y);
	//Backgroundbox->BackgoundColour = glm::vec3(0);
	CreateEditor();
	assert(Instance == nullptr);

	Instance = this;

}


Inspector::~Inspector()
{}

void Inspector::SetSelectedObject(IInspectable * newTarget)
{
	if (target != newTarget)
	{
		target = newTarget;
		CreateEditor();
	}
}

Inspector::InspectorPropery Inspector::CreateProperty(std::string name, ValueType type, void * Valueptr)
{
	Inspector::InspectorPropery NameProp;
	NameProp.name = name;
	NameProp.type = type;
	NameProp.ValuePtr = Valueptr;
	return NameProp;
}

Inspector::InspectorProperyGroup Inspector::CreatePropertyGroup(std::string name)
{
	Inspector::InspectorProperyGroup NameProp;
	NameProp.name = name;
	return NameProp;
}

void Inspector::Render()
{
	Backgroundbox->Render();
	if (SubWidgets.size() != 0)
	{
		for (int i = 0; i < SubWidgets.size(); i++)
		{
			SubWidgets[i]->Render();
		}
	}
}
void Inspector::ResizeView(int w, int h, int x, int y)
{
	UIWidget::ResizeView(w, h, x, y);
	Backgroundbox->ResizeView(w, h, x, y);
	UIUtils::ArrangeHorizontal(w, h, x, y, SubWidgets);
}
void Inspector::MouseMove(int x, int y)
{
	for (int i = 0; i < SubWidgets.size(); i++)
	{
		SubWidgets[i]->MouseMove(x, y);
	}
}

bool Inspector::MouseClick(int x, int y)
{
	bool Returnvalue = false;
	for (int i = 0; i < SubWidgets.size(); i++)
	{
		Returnvalue |= SubWidgets[i]->MouseClick(x, y);

	}
	return Returnvalue;
}
void Inspector::MouseClickUp(int x, int y)
{
	for (int i = 0; i < SubWidgets.size(); i++)
	{
		SubWidgets[i]->MouseClickUp(x, y);

	}
}
void Inspector::AddComponent()
{
	//__debugbreak();
	std::vector<std::string> ops;
	for (int i = 0; i < CompoenentRegistry::GetInstance()->GetCount(); i++)
	{
		ops.push_back(CompoenentRegistry::GetInstance()->GetNameById(i));
	}

	using std::placeholders::_1;
	UIManager::instance->CreateDropDown(ops, WidthScale, 0.2f, XoffsetScale, YoffsetScale, std::bind(&Inspector::AddComponentCallback, _1));
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
				Component* comp = CompoenentRegistry::CreateBaseComponent(CompoenentRegistry::BaseComponentTypes(i));
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
void Inspector::CreateEditor()
{
	if (SubWidgets.size() != 0)
	{
		for (int i = 0; i < SubWidgets.size(); i++)
		{
			delete SubWidgets[i];
		}
		SubWidgets.clear();
	}
	if (target != nullptr)
	{
		std::vector<Inspector::InspectorProperyGroup> Fields = target->GetInspectorFields();
		for (int i = 0; i < Fields.size(); i++)
		{
			UIPanel* Panel = new UIPanel(0, 0, 0, 0);
			for (int j = 0; j < Fields[i].SubProps.size(); j++)
			{
				UIWidget* newwidget = nullptr;
				switch (Fields[i].SubProps[j].type)
				{
				case Int:
				case Float:
				case String:
				case Vector:
				case Label:
				case Bool:
					newwidget = new UIEditField(Fields[i].SubProps[j].type, Fields[i].SubProps[j].name, Fields[i].SubProps[j].ValuePtr);
					break;
				default:
					break;
				}
				if (newwidget != nullptr)
				{
					newwidget->AligmentStruct.SizeMax = ItemHeight;
					Panel->AddSubWidget(newwidget);
				}
			}
			Panel->SetTitle(Fields[i].name);
			SubWidgets.push_back(Panel);

		}
		button = new UIButton(mwidth, 30, 0, 0);
		button->SetText("Add Component");
		button->BindTarget(std::bind(&Inspector::AddComponent, this));
		SubWidgets.push_back(button);
	}
	ResizeView(mwidth, mheight, X, Y);
	UIManager::UpdateBatches();
}
