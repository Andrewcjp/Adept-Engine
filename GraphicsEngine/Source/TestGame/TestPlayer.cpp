#include "TestPlayer.h"
#include "EngineHeader.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Components/Core_Components_inc.h"
TestPlayer::TestPlayer()
{

}


TestPlayer::~TestPlayer()
{}

void TestPlayer::InitComponent()
{

}

void TestPlayer::BeginPlay()
{
	RB = GetOwner()->GetComponent<RigidbodyComponent>();
}

void TestPlayer::Update(float delta)
{
	glm::vec3 TargetVel = RB->GetVelocity();
	TargetVel.z = 0.0f;
	TargetVel.x = 0.0f;
	if (Input::GetKey('a'))
	{
		TargetVel -= GetOwner()->GetTransform()->GetRight()* Speed;
	}
	if (Input::GetKey('d'))
	{
		TargetVel += GetOwner()->GetTransform()->GetRight()*Speed;
	}
	if (Input::GetKey('w'))
	{
		TargetVel += GetOwner()->GetTransform()->GetForward()* Speed;
	}
	if (Input::GetKey('s'))
	{
		TargetVel -= GetOwner()->GetTransform()->GetForward()*Speed;
	}
	if (Input::GetKeyDown(KeyCode::SPACE))
	{
		TargetVel += GetOwner()->GetTransform()->GetUp() * jumpHeight;
	}
	if (RB != nullptr)
	{
		RB->SetLinearVelocity(TargetVel);
	}
	if (CameraComponent::GetMainCamera() != nullptr)
	{
		glm::vec3 Pos = GetOwner()->GetTransform()->GetPos();
		CameraComponent::GetMainCamera()->SetPos(Pos);
	}
	if (Input::GetMouseButtonDown(0))
	{
		GameObject::Instantiate(glm::vec3(0,10,0));
	}
	if (/*Input::GetMouseButtonDown(0)*/true)
	{
		glm::vec2 axis = Input::GetMouseInputAsAxis();
		const glm::vec3 rot = GetOwner()->GetTransform()->GetEulerRot();
		glm::quat YRot = glm::quat(glm::radians(glm::vec3(0, -axis.x*LookSensitivty, 0)));
		GetOwner()->SetRotation(GetOwner()->GetTransform()->GetQuatRot()* YRot);
		glm::quat newrot = glm::quat(glm::radians(glm::vec3(axis.y*LookSensitivty, 0, 0)));

		glm::mat4 LocalMAtrix = GetOwner()->GetTransform()->GetModel();
		if (CameraObject)
		{
			glm::quat rotation = glm::quat(glm::toMat4(newrot)*glm::inverse(LocalMAtrix));
			rotation = glm::toMat4(rotation) *LocalMAtrix;


			CameraObject->GetTransform()->SetQrot(CameraObject->GetTransform()->GetQuatRot()* rotation);
			CameraComponent::GetMainCamera()->SetUpAndForward(CameraObject->GetTransform()->GetForward(), CameraObject->GetTransform()->GetUp());
		}
	//	Log::LogMessage(glm::to_string(CameraObject->GetTransform()->GetEulerRot()));
		Input::LockCursor(true);
		Input::SetCursorVisible(false);
	}
	else
	{
		Input::SetCursorVisible(true);
		Input::LockCursor(false);
	}
}

