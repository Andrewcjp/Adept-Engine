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
		group = UIInspectorBase::CreatePropertyGroup("Parameters");
		std::vector<ClassReflectionNode*> Nodes;
		//mat->GetAssetSet()->Validate();
		//mat->GetAssetSet()->InitReflection();
		//mat->GetAssetSet()->InitReflection();
		for (auto itor = mat->GetAssetSet()->BindMap.begin(); itor != mat->GetAssetSet()->BindMap.end(); itor++)
		{
			itor->second.InitReflection();
			if (itor->second.PropType == ShaderPropertyType::Texture)
			{
				ClassReflectionNode* node = new ClassReflectionNode(*itor->second.AccessReflection()->Data[2]);

				node->m_DisplayName = itor->first;
				Nodes.push_back(node);
			}
			else if (itor->second.PropType == ShaderPropertyType::Float)
			{
				ClassReflectionNode* FloatNode = new ClassReflectionNode(itor->first, MemberValueType::Float, new float());
				FloatNode->MapPropToFunctions([&](void* value) { /**(float*)value = ((MaterialAsset*)m_target)->GetAssetSet()->GetFloat(itor->first);*/ }, [&](void* value) { /*((MaterialAsset*)m_target)->GetAssetSet()->SetFloat(itor->first, *(float*)value);*/ });
				Nodes.push_back(FloatNode);
				//ClassReflectionNode* node = new ClassReflectionNode("", )
			}
			//	Nodes.push_back(itor->second.AccessReflection()->Data[i]);

		}
		group.Nodes = Nodes;
		Fields.push_back(group);

	}
}

void UIAssetInspector::OnInspectorFinishCreate(UIWidget* TabArea)
{

}

