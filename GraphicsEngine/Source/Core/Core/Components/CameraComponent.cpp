#include "CameraComponent.h"
#include "Editor/EditorWindow.h"
#include "Rendering/Core/Camera.h"
#include "Core/GameObject.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Core/Components/CameraComponent.generated.h"

CameraComponent::CameraComponent()
{}

void CameraComponent::InitComponent()
{
	MCamera = new Camera(GetOwner()->GetTransform()->GetPos(), 75.0f, 1.77f, 0.1f, 1000.0f);
	Transform* T = GetOwner()->GetTransform();
	MCamera->Sync(T);
}

void CameraComponent::SceneInitComponent()
{
	GetOwner()->GetScene()->AddCamera(MCamera);
}

CameraComponent::~CameraComponent()
{
	GetOwner()->GetScene()->RemoveCamera(MCamera);
	SafeDelete(MCamera);
}

void CameraComponent::BeginPlay()
{}

void CameraComponent::Update(float)
{
	if (MCamera != nullptr)
	{
		Transform* T = GetOwner()->GetTransform();
		MCamera->Sync(T);
	}
}

Camera * CameraComponent::GetMainCamera()
{
	return SceneRenderer::Get()->GetCurrentCamera();
}

void CameraComponent::ProcessSerialArchive(Archive * A)
{
	Component::ProcessSerialArchive(A);
}
#if WITH_EDITOR
void CameraComponent::GetInspectorProps(std::vector<InspectorProperyGroup>& props)
{
	InspectorProperyGroup group = Inspector::CreatePropertyGroup("Camera");
	//group.SubProps.push_back(Inspector::CreateProperty("test", Inspector::Float, nullptr));
	props.push_back(group);
}
#endif