#include "stdafx.h"
#include "LightComponent.h"
#include "../GameObject.h"
#include "../Editor/EditorWindow.h"
#include "../EngineGlobals.h"
#include "../Assets/Scene.h"
#include "CompoenentRegistry.h"
#include "../Assets/SerialHelpers.h"
#include "../Assets/SceneJSerialiser.h"
LightComponent::LightComponent() :Component()
{
	TypeID = CompoenentRegistry::BaseComponentTypes::LightComp;
}

LightComponent::~LightComponent()
{

	GetOwner()->GetScene()->RemoveLight(MLight);

	delete MLight;
}

void LightComponent::InitComponent()
{
	MLight = new Light(GetOwner()->GetTransform()->GetPos(), DefaultIntesity, DefaultType, glm::vec3(1), DefaultShadow);
	MLight->SetDirection(GetOwner()->GetTransform()->GetForward());

}

void LightComponent::BeginPlay()
{}

void LightComponent::Update(float)
{}

void LightComponent::SetLightType(Light::LightType type)
{
	MLight->SetLightType(type);
}

void LightComponent::SetIntensity(float amt)
{
	MLight->SetIntesity(amt);
	intensity = amt;
}

void LightComponent::SetShadow(bool Shadow)
{
	MLight->SetShadow(Shadow);
}

void LightComponent::SetLightColour(glm::vec3)
{

}

Light * LightComponent::Internal_GetLightPtr()
{
	return MLight;
}

void LightComponent::OnTransformUpdate()
{
	MLight->SetPostion(GetOwner()->GetTransform()->GetPos());
	MLight->SetDirection(GetOwner()->GetTransform()->GetForward());
	if (GetOwner()->GetScene())
	{
		GetOwner()->GetScene()->StaticSceneNeedsUpdate = true;
	}
}

void LightComponent::SceneInitComponent()
{
	GetOwner()->GetScene()->AddLight(MLight);
	Shadow = MLight->GetDoesShadow();

}

void LightComponent::PostChangeProperties()
{
	SetShadow(Shadow);
	SetIntensity(intensity);
	if (GetOwner()->GetScene())
	{
		GetOwner()->GetScene()->StaticSceneNeedsUpdate = true;
	}
}

void LightComponent::Serialise(rapidjson::Value & v)
{
	Component::Serialise(v);
	SerialHelpers::addLiteral(v, *SceneJSerialiser::jallocator, "Intensity", MLight->GetIntesity());
	SerialHelpers::addLiteral(v, *SceneJSerialiser::jallocator, "LightType", MLight->GetType());
	SerialHelpers::addBool(v, *SceneJSerialiser::jallocator, "LightShadow", MLight->GetDoesShadow());

}

void LightComponent::Deserialise(rapidjson::Value & v)
{
	for (auto& it = v.MemberBegin(); it != v.MemberEnd(); it++)
	{
		std::string key = (it->name.GetString());
		if (key == "Intensity")
		{
			DefaultIntesity = it->value.GetFloat();
			//MLight->SetIntesity(it->value.GetFloat());
		}
		if (key == "LightType")
		{
			DefaultType = (Light::LightType)it->value.GetInt();
			//MLight->SetLightType((Light::LightType)it->value.GetInt());
		}
		if (key == "LightShadow")
		{
			DefaultShadow = it->value.GetBool();
			//MLight->SetLightType((Light::LightType)it->value.GetInt());
		}
	}
}
void LightComponent::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("Light");
	group.SubProps.push_back(Inspector::CreateProperty("test", Inspector::Bool, &Shadow));
	group.SubProps.push_back(Inspector::CreateProperty("Intensity", Inspector::Float, &intensity));
	props.push_back(group);
}