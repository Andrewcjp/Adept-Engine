
#include "FreeLookComponent.h"
#include "Rendering/Core/Camera.h"
#include "Core/Input/Input.h"
#include "Core/GameObject.h"
#include "Core/Components/CameraComponent.h"
#include "Editor/EditorCameraController.h"
FreeLookComponent::FreeLookComponent()
{
	controller = new EditorCameraController();
}


FreeLookComponent::~FreeLookComponent()
{}

void FreeLookComponent::InitComponent()
{

}

void FreeLookComponent::BeginPlay()
{
	CameraComp = GetOwner()->GetComponent<CameraComponent>();
	if (CameraComp != nullptr)
	{
		Cam = CameraComp->GetMainCamera();
	}
}

void FreeLookComponent::Update(float delta)
{
	Transform* Form = GetOwner()->GetTransform();
	controller->Target = Form;
	controller->Update();
}
