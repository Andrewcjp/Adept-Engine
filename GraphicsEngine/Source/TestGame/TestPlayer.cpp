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
		TargetVel += GetOwner()->GetTransform()->GetRight()* Speed;
	}
	if (Input::GetKey('d'))
	{
		TargetVel -= GetOwner()->GetTransform()->GetRight()*Speed;
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
		TargetVel -= GetOwner()->GetTransform()->GetUp() * jumpHeight;
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
		glm::vec2 axis = Input::GetMouseInputAsAxis();

#if 0	
		Form->RotateAboutAxis(glm::vec3(0, 1, 0), axis.y*LookSensitivty);
		Form->RotateAboutAxis(Form->GetRight(), -axis.x*LookSensitivty);
#else 
		//CameraComponent::GetMainCamera()->RotateY(axis.x*LookSensitivty);
		//GetOwner()->GetTransform()->RotateAboutAxis(GetOwner()->GetTransform()->GetUp(), axis.x*LookSensitivty);
#if 0
		glm::vec3 rotation = GetOwner()->GetTransform()->GetEulerRot();
		rotation.xz = 0;
		//rotation.z = 0.0;
		rotation.y += 1;// 
		GetOwner()->GetTransform()->SetEulerRot(rotation);
#else
		GetOwner()->GetTransform()->RotateAboutAxis(glm::vec3(0, 1, 0), -axis.x*(1.0 / 100.0f));
		//GetOwner()->GetTransform()->RotateAboutAxis(GetOwner()->GetTransform()->GetRight(), axis.y*(1.0 / 100.0f));
#endif

		//GetOwner()->GetTransform()->SetEulerRot(glm::vec3(0, 90, 0));
		const glm::vec3 rot = GetOwner()->GetTransform()->GetEulerRot();
		Log::LogMessage(glm::to_string(rot));
		//Log::LogMessage(glm::to_string(GetOwner()->GetTransform()->GetForward()));
		//GetOwner()->
		CameraComponent::GetMainCamera()->SetUpAndForward(GetOwner()->GetTransform()->GetForward(), GetOwner()->GetTransform()->GetUp());
		CameraComponent::GetMainCamera()->Pitch(axis.y*LookSensitivty);
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

