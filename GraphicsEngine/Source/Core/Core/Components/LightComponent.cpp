#include "stdafx.h"
#include "LightComponent.h"
#include "Core/GameObject.h"
#include "Editor/EditorWindow.h"

#include "Core/Assets/Scene.h"
#include "CompoenentRegistry.h"
#include "Core/Assets/Archive.h"
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
	MLight = new Light(GetOwner()->GetTransform()->GetPos(), DefaultIntesity, DefaultType, DefaultColour, DefaultShadow);
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

void LightComponent::SetLightColour(glm::vec3 value)
{
	MLight->m_lightColor = value;
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
void LightComponent::ProcessSerialArchive(Archive * A)
{
	Component::ProcessSerialArchive(A);
	ArchiveProp(DefaultIntesity);
	ArchiveProp(DefaultShadow);
	ArchiveProp(DefaultColour);
	ArchiveProp_Enum(DefaultType, Light::LightType);
}

void LightComponent::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("Light");
	group.SubProps.push_back(Inspector::CreateProperty("test", Inspector::Bool, &Shadow));
	group.SubProps.push_back(Inspector::CreateProperty("Intensity", Inspector::Float, &intensity));
	props.push_back(group);
}


