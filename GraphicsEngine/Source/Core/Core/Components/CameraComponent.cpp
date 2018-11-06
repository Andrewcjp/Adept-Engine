
#include "CameraComponent.h"
#include "Editor/EditorWindow.h"
#include "Rendering/Core/Camera.h"
#include "CompoenentRegistry.h"
#include "Core/GameObject.h"
#include "Core/Assets/Scene.h"

CameraComponent::CameraComponent()
{
	TypeID = CompoenentRegistry::BaseComponentTypes::CameraComp;
}
void CameraComponent::InitComponent()
{
	MCamera = new Camera(GetOwner()->GetTransform()->GetPos(), 75.0f, 1.77f, 0.1f, 1000.0f);

}
void CameraComponent::SceneInitComponent()
{
	GetOwner()->GetScene()->AddCamera(MCamera);
}
CameraComponent::~CameraComponent()
{

	GetOwner()->GetScene()->RemoveCamera(MCamera);
	delete MCamera;
}

void CameraComponent::BeginPlay()
{
}

void CameraComponent::Update(float )
{
	if (MCamera != nullptr)
	{
		Transform* T = GetOwner()->GetTransform();
		MCamera->SetPos(T->GetPos());
		//MCamera->SetUpAndForward(T->GetForward(), T->GetUp());
	}
}

Camera * CameraComponent::GetMainCamera()
{
	return BaseWindow::GetCurrentCamera();
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