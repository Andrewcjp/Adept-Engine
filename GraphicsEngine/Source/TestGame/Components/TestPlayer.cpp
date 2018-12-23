#include "TestPlayer.h"
#include "Core/Components/Core_Components_inc.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Editor/EditorWindow.h"
#include "Health.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "WeaponManager.h"
TestPlayer::TestPlayer()
{}

TestPlayer::~TestPlayer()
{}

void TestPlayer::InitComponent()
{}

void TestPlayer::OnCollide(CollisonData data)
{}

std::string TestPlayer::GetInfoString()
{
	std::stringstream ss;
	ss << Manager->GetCurrentWeaponinfoString() << " Health " << Mhealth->GetCurrentHealth();
	return ss.str();
}

void TestPlayer::CheckForGround()
{
	IsGrounded = false;
	RayHit hit;
	glm::vec3 down = -GetOwner()->GetTransform()->GetRight();
	std::vector<RigidBody*> IgnoreActors;
	IgnoreActors.push_back(RB->GetActor());
	if (Engine::GetPhysEngineInstance()->RayCastScene(GetOwner()->GetPosition(), down, 3.0f, &hit, IgnoreActors))
	{
		const float angle = glm::degrees(glm::angle(glm::vec3(0, 1, 0), hit.Normal));
		if (angle < MaxWalkableAngle)
		{
			IsGrounded = true;
		}
	}

	/*Frontblocked = false;
	if (Engine::GetPhysEngineInstance()->RayCastScene(GetOwner()->GetPosition() + GetOwner()->GetTransform()->GetForward() *1.5f, down, 3, &hit, IgnoreActors))
	{
		const float angle = glm::degrees(glm::angle(glm::vec3(0, 1, 0), hit.Normal));
		if (angle > MaxWalkableAngle)
		{
			Frontblocked = true;
		}
	}*/
}

void TestPlayer::BeginPlay()
{
	Mhealth = GetOwner()->GetComponent<Health>();
	Manager = GetOwner()->GetComponent<WeaponManager>();
	RB = GetOwner()->GetComponent<RigidbodyComponent>();
	RB->IsKineimatic = true;
	glm::quat newrot = glm::quat(glm::radians(glm::vec3(90, 90, 0)));
	CameraObject->SetParent(GetOwner());
	CameraObject->GetTransform()->SetLocalRotation(newrot);
	CameraObject->GetTransform()->SetLocalPosition(glm::vec3(2, 0, 0));
	const glm::vec3 rot = GetOwner()->GetTransform()->GetEulerRot();
}

static ConsoleVariable Sensitivity("sensitivity", 1.0f, ECVarType::ConsoleOnly);
void TestPlayer::Update(float delta)
{
	const float EyeHeight = 1.9f;
#if WITH_EDITOR
	if (EditorWindow::GetInstance()->IsEditorEjected())
	{
		return;
	}
#endif
	CheckForGround();
	UpdateMovement(delta);

	glm::vec2 axis = Input::GetMouseInputAsAxis();
	axis *= Sensitivity.GetFloatValue();
	const glm::vec3 rot = GetOwner()->GetTransform()->GetEulerRot();
	glm::quat YRot = glm::quat(glm::radians(glm::vec3(rot.x + -axis.x*LookSensitivty, 0, 90)));
	GetOwner()->SetRotation(YRot);
	glm::quat newrot = glm::quat(glm::radians(glm::vec3(axis.y*LookSensitivty, 0, 0)));
	//	Log::LogMessage(glm::to_string(axis) + " axis");
	if (CameraObject)
	{
		CameraObject->GetTransform()->SetLocalRotation(newrot);

		CameraObject->GetTransform()->SetLocalPosition(glm::vec3(EyeHeight, 0, 0));
		CameraComponent::GetMainCamera()->SetUpAndForward(CameraObject->GetTransform()->GetForward(), CameraObject->GetTransform()->GetUp());
		CameraComponent::GetMainCamera()->SetPos(CameraObject->GetTransform()->GetPos());
	}
	Input::SetCursorState(true, false);
}

void TestPlayer::UpdateMovement(float delta)
{
	//return;
	glm::vec3 TargetVel = glm::vec3(0, 0, 0);
	glm::vec3 right = glm::vec3(1, 0, 0);
	glm::vec3 fwd = glm::vec3(0, 0, 1);
	float Speed = 1;
	if (!IsGrounded)
	{
		Speed = AirSpeedFactor;
	}

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
		TargetVel -= fwd * Speed;
	}
	if (Input::GetKey('s'))
	{
		TargetVel += fwd * Speed;
	}
	float friction = 0.1f;
	if (TargetVel == glm::vec3(0))
	{
		friction = 0.5f;
	}
	if (IsGrounded)
	{
		RelativeSpeed -= RelativeSpeed * 20.0f*friction*delta;
	}
	else
	{
		RelativeSpeed -= RelativeSpeed * 5.0f*friction*delta;
	}
	glm::vec3 CurrentVel = RB->GetVelocity();
	RelativeSpeed += (TargetVel*Acceleration*delta);
	RelativeSpeed = glm::clamp(RelativeSpeed, -glm::vec3(MaxSpeed), glm::vec3(MaxSpeed));
	glm::vec3 NewVel = (RelativeSpeed.z*GetOwner()->GetTransform()->GetForward()) + (RelativeSpeed.x*GetOwner()->GetTransform()->GetUp());
	NewVel.y = CurrentVel.y;
	if (RB != nullptr)
	{
#if 1
		const glm::vec3 tVel = NewVel /*+ ExtraVel*/;
		glm::vec3 correction = tVel - RB->GetVelocity();
		correction.y = 0.0f;
#if WITH_EDITOR
		Log::LogTextToScreen("Correction: " + glm::to_string(correction));
#endif
		RB->GetActor()->AddForce(correction * (IsGrounded ? 10 : 2.5f));
#else
		RB->SetLinearVelocity(NewVel);
#endif
	}
	if (Input::GetKeyDown(KeyCode::SPACE) && IsGrounded)
	{
		RB->GetActor()->AddForce((glm::vec3(0, 1, 0) * 10) / delta);
	}
#if WITH_EDITOR
	Log::LogTextToScreen("Speed: " + glm::to_string(RB->GetVelocity()));
#endif
}

