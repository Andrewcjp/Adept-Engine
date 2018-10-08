#include "TestPlayer.h"
#include "EngineHeader.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Components/Core_Components_inc.h"
#include "Projectile.h"
#include "Editor/EditorWindow.h"
#include <algorithm>
TestPlayer::TestPlayer()
{
	Acceleration = 20.0f;
}


TestPlayer::~TestPlayer()
{}

void TestPlayer::InitComponent()
{

}

void TestPlayer::OnCollide(CollisonData data)
{

}
void TestPlayer::CheckForGround()
{
	IsGrounded = false;
	RayHit hit;
	glm::vec3 down = -GetOwner()->GetTransform()->GetUp();
	if (Engine::GetPhysEngineInstance()->RayCastScene(GetOwner()->GetPosition() - down * 2.5, down, 2, &hit))
	{
		IsGrounded = true;
		Log::LogMessage("G");
	}
}
void TestPlayer::BeginPlay()
{
	RB = GetOwner()->GetComponent<RigidbodyComponent>();
}

void TestPlayer::Update(float delta)
{
	if (EditorWindow::GetInstance()->IsEditorEjected())
	{
		return;
	}
	CheckForGround();
	if (IsGrounded)
	{
		UpdateMovement(delta);
	}
	if (CameraComponent::GetMainCamera() != nullptr)
	{
		glm::vec3 Pos = GetOwner()->GetTransform()->GetPos();
		CameraComponent::GetMainCamera()->SetPos(Pos);
	}

	glm::vec2 axis = Input::GetMouseInputAsAxis();
	const glm::vec3 rot = GetOwner()->GetTransform()->GetEulerRot();
	glm::quat YRot = glm::quat(glm::radians(glm::vec3(0, -axis.x*LookSensitivty, 0)));
	GetOwner()->SetRotation(GetOwner()->GetTransform()->GetQuatRot()* YRot);
	glm::quat newrot = glm::quat(glm::radians(glm::vec3(axis.y*LookSensitivty, 0, 0)));
	if (CameraObject)
	{
#if 1
		glm::mat4 LocalMAtrix = GetOwner()->GetTransform()->GetModel();
		glm::quat rotation = glm::quat(glm::toMat4(newrot)*glm::inverse(LocalMAtrix));
		rotation = glm::toMat4(rotation) *LocalMAtrix;


		CameraObject->GetTransform()->SetQrot(CameraObject->GetTransform()->GetQuatRot()* rotation);
#else
		CameraObject->GetTransform()->SetLocalRotation(newrot);
#endif
		CameraComponent::GetMainCamera()->SetUpAndForward(CameraObject->GetTransform()->GetForward(), CameraObject->GetTransform()->GetUp());
	}
	Input::SetCursorState(true, false);
}

void TestPlayer::UpdateMovement(float delta)
{
	glm::vec3 TargetVel = glm::vec3(0, 0, 0);
	glm::vec3 right = glm::vec3(1, 0, 0);
	glm::vec3 fwd = glm::vec3(0, 0, 1);
	Speed = 1;
	if (Input::GetKey('a'))
	{
		TargetVel -= right * Speed;
	}
	if (Input::GetKey('d'))
	{
		TargetVel += right * Speed;
	}
	if (Input::GetKey('w'))
	{
		TargetVel += fwd * Speed;
	}
	if (Input::GetKey('s'))
	{
		TargetVel -= fwd * Speed;
	}
	float friction = 0.1f;
	if (TargetVel == glm::vec3(0))
	{
		friction = 0.5f;
	}
	RelativeSpeed -= RelativeSpeed * 20.0f*friction*delta;
	glm::vec3 CurrentVel = RB->GetVelocity();
	RelativeSpeed += (TargetVel*Acceleration*delta);
	RelativeSpeed = glm::clamp(RelativeSpeed, -glm::vec3(MaxSpeed), glm::vec3(MaxSpeed));
	glm::vec3 NewVel = (RelativeSpeed.z*GetOwner()->GetTransform()->GetForward()) + (RelativeSpeed.x*GetOwner()->GetTransform()->GetRight());
	NewVel.y = RB->GetVelocity().y;
	if (Input::GetKeyDown(KeyCode::SPACE))
	{
		NewVel += GetOwner()->GetTransform()->GetUp() * jumpHeight;
	}
	if (RB != nullptr)
	{
		RB->SetLinearVelocity(NewVel);
	}
}

