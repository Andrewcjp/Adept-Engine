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
Inspector::Inspector(int w, int h, int x, int y)
	:UIWidget(w, h, x, y)
{
	Backgroundbox = new UIBox(w, h, x, y);
	//Backgroundbox->BackgoundColour = glm::vec3(0);
	CreateEditor();
}


Inspector::~Inspector()
{
}

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

void Inspector::MouseClick(int x, int y)
{
	for (int i = 0; i < SubWidgets.size(); i++)
	{
		SubWidgets[i]->MouseClick(x, y);

	}
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
}
void Inspector::CreateEditor()
{
	if (target == nullptr)
	{
		return;
	}
	if (SubWidgets.size() != 0)
	{
		for (int i = 0; i < SubWidgets.size(); i++)
		{
			delete SubWidgets[i];
		}
		SubWidgets.clear();
	}
	std::vector<Inspector::InspectorProperyGroup> Fields = target->GetInspectorFields();
	for (int i = 0; i < Fields.size(); i++)
	{
		UIPanel* Panel = new UIPanel(0,0,0,0);
		
		for (int j = 0; j < Fields[i].SubProps.size(); j++)
		{
			UIWidget* newwidget = nullptr;
			switch (Fields[i].SubProps[j].type)
			{
			case Int:
			case Float:
			case String:
			case Label:
				newwidget = new UIEditField(Fields[i].SubProps[j].type, Fields[i].SubProps[j].name, Fields[i].SubProps[j].ValuePtr);
				break;
			default:
				break;
			}
			if (newwidget != nullptr)
			{
				newwidget->AligmentStruct.SizeMax = 0.03f;
				Panel->AddSubWidget(newwidget);
			}
		}
		Panel->SetTitle(Fields[i].name);
		SubWidgets.push_back(Panel);
		
	}
	UIButton* button = new UIButton(mwidth, 30, 0, 0);
	button->SetText("Add Component");
	button->BindTarget(std::bind(&Inspector::AddComponent, this));
	SubWidgets.push_back(button);
	ResizeView(mwidth, mheight, X, Y);
	UIManager::UpdateBatches();
}
