#include "UIAssetInspector.h"
#include "Core/Reflection/IReflect.h"
#include "Core/Assets/Asset types/MaterialAsset.h"

UIAssetInspector* UIAssetInspector::Instance = nullptr;

UIAssetInspector::UIAssetInspector() :UIInspectorBase(0, 0, 0, 0)
{
	ensure(Instance == nullptr);
	Instance = this;
	SetName("Asset Inspector");
}

void UIAssetInspector::SetTarget(BaseAsset * a)
{
	if (m_target != a)
	{
		m_target = a;
		CreateEditor();
	}
}

void UIAssetInspector::OnPopulateFields()
{
	if (m_target == nullptr)
	{
		return;
	}
	if (m_target->GetId() == MaterialAsset::TYPEID)
	{
		MaterialAsset* mat = (MaterialAsset*)m_target;
		InspectorProperyGroup group = UIInspectorBase::CreatePropertyGroup("Material");
		group.Nodes = m_target->AccessReflection()->Data;
		Fields.push_back(group);
	}
}

void UIAssetInspector::OnInspectorFinishCreate(UIWidget* TabArea)
{

}

