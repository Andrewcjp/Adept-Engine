#include "stdafx.h"
#include "CameraComponent.h"
#include "../EngineGlobals.h"
#include "../Editor/EditorWindow.h"
#include "../Rendering/Core/Camera.h"
#include "CompoenentRegistry.h"
CameraComponent::CameraComponent()
{
	TypeID = CompoenentRegistry::BaseComponentTypes::CameraComp;
}
void CameraComponent::InitComponent()
{
	MCamera = new Camera(GetOwner()->GetTransform()->GetPos(), 75.0f, 1.77f, 0.1f, 1000.0f);
#if WITH_EDITOR
	EditorWindow::GetCurrentScene()->AddCamera(MCamera);
#else
	//todo: this!
#endif
}

CameraComponent::~CameraComponent()
{
#if WITH_EDITOR
	EditorWindow::GetCurrentScene()->RemoveCamera(MCamera);
#else
	//todo: this!
#endif
	delete MCamera;
}

void CameraComponent::BeginPlay()
{
}

void CameraComponent::Update(float delta)
{
}

void CameraComponent::Serialise(rapidjson::Value & v)
{
	Component::Serialise(v);
}

void CameraComponent::Deserialise(rapidjson::Value & v)
{
}
void CameraComponent::GetInspectorProps(std::vector<Inspector::InspectorProperyGroup>& props)
{
	Inspector::InspectorProperyGroup group = Inspector::CreatePropertyGroup("Camera");
	//group.SubProps.push_back(Inspector::CreateProperty("test", Inspector::Float, nullptr));
	props.push_back(group);
}