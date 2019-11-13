
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
#if WITH_EDITOR
Inspector* Inspector::Instance = nullptr;
Inspector::Inspector(int w, int h, int x, int y)
	:UIWidget(w, h, x, y)
{
	Backgroundbox = new UIBox(w, h, x, y);
	//Backgroundbox->BackgoundColour = glm::vec3(0);
	CreateEditor();
	ensure(Instance == nullptr);
	AddChild(Backgroundbox);
	Instance = this;

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
	for (int i = 0; i < ComponentRegistry::GetInstance()->GetCount(); i++)
	{
		ops.push_back(ComponentRegistry::GetInstance()->GetNameById(i));
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
void Inspector::CreateEditor()
{
	if (SubWidgets.size() != 0)
	{
		for (int i = 0; i < SubWidgets.size(); i++)
		{
			RemoveChild(SubWidgets[i]);
			delete SubWidgets[i];
		}
		SubWidgets.clear();
	}
	if (target != nullptr)
	{
		std::vector<InspectorProperyGroup> Fields = target->GetInspectorFields();
		for (int i = 0; i < Fields.size(); i++)
		{
			UIPanel* Panel = new UIPanel(0, 0, 0, 0);
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
					newwidget->AligmentStruct.SizeMax = ItemHeight;
					Panel->AddSubWidget(newwidget);
				}
			}
			Panel->SetTitle(Fields[i].name);
			SubWidgets.push_back(Panel);
			AddChild(Panel);
		}
		button = new UIButton(mwidth, 30, 0, 0);
		button->SetRootSpaceSize(250, 30, 0, 0);
		button->SetText("Add Component");
		button->BindTarget(std::bind(&Inspector::AddComponent, this));
		AddChild(button);
		SubWidgets.push_back(button);
	}
	ResizeView(mwidth, mheight, X, Y);
	UIManager::UpdateBatches();
}
#endif