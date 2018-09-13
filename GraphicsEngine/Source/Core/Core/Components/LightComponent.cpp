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
	MLight = new Light(GetOwner()->GetTransform()->GetPos(), CurrentIntensity, CurrentType, CurrentColour, DoesShadow);
	MLight->SetDirection(GetOwner()->GetTransform()->GetForward());
}

void LightComponent::BeginPlay()
{}

void LightComponent::Update(float)
{}

void LightComponent::SetLightType(Light::LightType type)
{
	MLight->SetLightType(type);
	CurrentType = type;
}

void LightComponent::SetIntensity(float amt)
{
	MLight->SetIntesity(amt);
	CurrentIntensity = amt;
}

void LightComponent::SetShadow(bool Shadow)
{
	MLight->SetShadow(Shadow);
}

void LightComponent::SetLightColour(glm::vec3 value)
{
	MLight->m_lightColor = value;
	CurrentColour = value;
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
	DoesShadow = MLight->GetDoesShadow();

}

void LightComponent::PostChangeProperties()
{
	SetShadow(DoesShadow);
	SetIntensity(CurrentIntensity);
	if (GetOwner()->GetScene())
	{
		GetOwner()->GetScene()->StaticSceneNeedsUpdate = true;
	}
}
void LightComponent::ProcessSerialArchive(Archive * A)
{
	Component::ProcessSerialArchive(A);
	ArchiveProp(CurrentIntensity);
	ArchiveProp(DoesShadow);
	ArchiveProp(CurrentColour);
	ArchiveProp_Enum(CurrentType, Light::LightType);
}

void LightComponent::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("Light");
	group.SubProps.push_back(Inspector::CreateProperty("test", Inspector::Bool, &DoesShadow));
	group.SubProps.push_back(Inspector::CreateProperty("Intensity", Inspector::Float, &CurrentIntensity));
	props.push_back(group);
}


