
#include "LightComponent.h"
#include "Core/GameObject.h"
#include "Editor/EditorWindow.h"
#include "Core/Assets/Scene.h"
#include "Core/Assets/Archive.h"
#include "Core/Components/LightComponent.generated.h"

LightComponent::LightComponent() :Component()
{
	m_DisplayName = "Light Component";
	CALL_CONSTRUCTOR();
}

LightComponent::~LightComponent()
{
	GetOwner()->GetScene()->RemoveLight(MLight);
	SafeDelete(MLight);
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

void LightComponent::SetLightType(ELightType::Type type)
{
	MLight->SetLightType(type);
	CurrentType = type;
}

void LightComponent::SetDistance(float amt)
{
	MLight->Distance = amt;
	Distance = amt;
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
	OnTransformUpdate();
	SetShadow(DoesShadow);
	SetIntensity(CurrentIntensity);
	SetDistance(Distance);
}
#if WITH_EDITOR
void LightComponent::PostChangeProperties()
{
	SetShadow(DoesShadow);
	SetIntensity(CurrentIntensity);
	SetDistance(Distance);
	MLight->m_lightColor = CurrentColour;
	if (GetOwner()->GetScene())
	{
		GetOwner()->GetScene()->StaticSceneNeedsUpdate = true;
	}
}

void LightComponent::OnPropertyUpdate(ClassReflectionNode* Node)
{
	PostChangeProperties();
}

#endif

