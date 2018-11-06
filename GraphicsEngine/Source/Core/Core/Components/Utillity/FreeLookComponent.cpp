
#include "FreeLookComponent.h"
#include "Rendering/Core/Camera.h"
#include "Core/Input/Input.h"
#include "Core/GameObject.h"
#include "Core/Components/CameraComponent.h"
FreeLookComponent::FreeLookComponent()
{

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
	if (Input::GetVKey(0x0))
	{
		TranslateSpeed = FastTranslateSpeed;
	}
	else
	{
		TranslateSpeed = BaseTranslateSpeed;
	}

	glm::vec3 Forward = Cam->GetForward() * TranslateSpeed;
	glm::vec3 Right = Cam->GetRight() * TranslateSpeed;
	glm::vec3 Up = Cam->GetUp() * TranslateSpeed;
	if (Input::GetKey('w'))
	{
		Form->TranslatePos(Forward*delta);
	}
	if (Input::GetKey('s'))
	{
		Form->TranslatePos(-Forward *delta);
	}
	if (Input::GetKey('d'))
	{
		Form->TranslatePos(Right*delta);
	}
	if (Input::GetKey('a'))
	{
		Form->TranslatePos(-Right * delta);
	}
	if (Input::GetKey('e'))
	{
		Form->TranslatePos(Up*delta);
	}
	if (Input::GetKey('q'))
	{
		Form->TranslatePos(-Up * delta);
	}
	if (Input::GetMouseButtonDown(0) && Cam != nullptr)
	{
		glm::vec2 axis = Input::GetMouseInputAsAxis();

#if 0	
		Form->RotateAboutAxis(glm::vec3(0, 1, 0), axis.y*LookSensitivty);
		Form->RotateAboutAxis(Form->GetRight(), -axis.x*LookSensitivty);

#else 
		Cam->RotateY(axis.x*LookSensitivty);
		Cam->Pitch(axis.y*LookSensitivty);
#endif


		Input::LockCursor(true);
		Input::SetCursorVisible(false);
	}
	else
	{
		Input::SetCursorVisible(true);
		Input::LockCursor(false);
	}
}




//void FreeLookComponent::RotateY(float angle)
//{
//
//	static const glm::vec3 UP(0.0f, 1.0f, 0.0f);
//	if (UseLeftHanded)
//	{
//		angle = -angle;
//	}
//	glm::mat4 irotation = glm::rotate(angle, UP);
//
//	forward = glm::vec3(glm::normalize(irotation * glm::vec4(forward, 0.0)));
//	up = glm::vec3(glm::normalize(irotation * glm::vec4(up, 0.0)));
//}