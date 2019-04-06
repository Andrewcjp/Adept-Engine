
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
#if 0
	MCamera->SetUpAndForward(GetOwner()->GetTransform()->GetForward(), glm::vec3(0, 1, 0));
#else
	//Hack!
	MCamera->SetPos(glm::vec3(0, 2, 20));
	MCamera->SetUpAndForward(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	//End hack
#endif
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
		MCamera->SetPos(T->GetPos());
		//#Physx todo: remove this hack !
		if (AllowRotSync)
		{
			MCamera->SetUpAndForward(T->GetForward(), T->GetUp());
		}
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