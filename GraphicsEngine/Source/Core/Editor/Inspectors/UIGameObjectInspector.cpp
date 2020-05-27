#include "UIGameObjectInspector.h"
#include "UI/CompoundWidgets/UIButton.h"
#include "UI/UIManager.h"
#include "Core/Reflection/ObjectLibrary.h"
#include "Core/GameObject.h"
#include "Core/Components/Component.h"
#include "UI/Core/UITransform.h"
#include "UI/Core/UIWidget.h"
UIGameObjectInspector* UIGameObjectInspector::Instance = nullptr;
UIGameObjectInspector::UIGameObjectInspector() : UIInspectorBase(0, 0, 0, 0)
{
	ensure(Instance == nullptr);
	Instance = this;
}

UIGameObjectInspector * UIGameObjectInspector::Get()
{
	return Instance;
}

void UIGameObjectInspector::OnPopulateFields()
{
	if (target != nullptr)
	{
		Fields = target->GetInspectorFields();
	}
}

void UIGameObjectInspector::OnInspectorFinishCreate(UIWidget* TabArea)
{
	if (target == nullptr)
	{
		return;
	}
	button = new UIButton(mwidth, 30, 0, 0);
	button->SetRootSpaceSize(250, 50, 0, 0);
	button->SetText("Add Component");
	button->GetTransfrom()->SetAnchourPoint(EAnchorPoint::Top);
	button->BindTarget(std::bind(&UIGameObjectInspector::AddComponent, this));
	button->AligmentStruct.SizeMax = 50;
	TabArea->AddChild(button);
}


void UIGameObjectInspector::AddComponentCallback(int i)
{
	//close dropdown
	if (Instance != nullptr)
	{
		if (Instance->target != nullptr)
		{
			GameObject* t = (GameObject*)Instance->target;
			if (t != nullptr)
			{
				Component* comp = ObjectLibrary::Create<Component>(Instance->ListIds[i]);
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

void UIGameObjectInspector::AddComponent()
{
	ListIds.clear();
	std::vector<std::string> ops;
	std::vector<const ClassType*> types = ObjectLibrary::GetAllItemsOfType(0);
	for (int i = 0; i < types.size(); i++)
	{
		//todo: use filter by type
		if (types[i]->Name.find("omp") != std::string::npos && types[i]->Name != "Component")
		{
			ops.push_back(types[i]->Name);
			ListIds.push_back(types[i]->TypeId);
		}
	}
	using std::placeholders::_1;
	UIManager::instance->CreateDropDown(ops, button->GetTransfrom()->GetSizeRootSpace().x, 200, button->GetTransfrom()->GetPositionForWidgetRootSpace().x, button->GetTransfrom()->GetPositionForWidgetRootSpace().y, std::bind(&UIGameObjectInspector::AddComponentCallback,this, _1));
}

void UIGameObjectInspector::SetSelectedObject(IEdtiorInspectable * newTarget)
{
	if (target != newTarget)
	{
		target = newTarget;
		CreateEditor();
	}
}