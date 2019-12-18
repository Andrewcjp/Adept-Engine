#include "BleedOutPlayer.h"
#include "Core/Components/Core_Components_inc.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Editor/EditorWindow.h"
#include "Health.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "WeaponManager.h"
#include "Core/Utils/DebugDrawers.h"
#include "Audio/AudioEngine.h"
#include <iomanip>
#include "Components/BleedOutPlayer.generated.h"

BleedOutPlayer::BleedOutPlayer()
{}

BleedOutPlayer::~BleedOutPlayer()
{}

void BleedOutPlayer::InitComponent()
{}

void BleedOutPlayer::OnCollide(CollisonData data)
{


}

std::string BleedOutPlayer::GetInfoString()
{
	std::stringstream ss;
	ss << Manager->GetCurrentWeaponinfoString() << " Health " << std::fixed << std::setprecision(1) << Mhealth->GetCurrentHealth();
	ss << " Time to Death " << std::setprecision(2) << SecondsLeft << "s ";
	return ss.str();
}

glm::vec3 BleedOutPlayer::GetColour()
{
	float H = Mhealth->GetCurrentHealth();
	if (H >= 50)
	{
		return glm::vec3(1);
	}
	if (H <= 1)
	{
		return glm::vec3(1, 0, 0);
	}
	return glm::mix(glm::vec3(1, 0, 0), glm::vec3(1), H / 50);
}

float BleedOutPlayer::GetPlayerHealth()
{
	return Mhealth->GetCurrentHealth();
}

void BleedOutPlayer::CheckForGround()
{
	IsGrounded = false;
	RayHit hit;
	glm::vec3 down = -GetOwner()->GetTransform()->GetUp();
	std::vector<RigidBody*> IgnoreActors;
	IgnoreActors.push_back(RB->GetActor());
	if (Engine::GetPhysEngineInstance()->RayCastScene(GetOwner()->GetPosition(), -down, 2.5f, &hit, IgnoreActors))
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
	if (IsGrounded && !LastFrameGrounded)
	{
		AudioEngine::PostEvent("Land", GetOwner());
	}
}

void BleedOutPlayer::BeginPlay()
{
	Mhealth = GetOwner()->GetComponent<Health>();
	Manager = GetOwner()->GetComponent<WeaponManager>();
	RB = GetOwner()->GetComponent<RigidbodyComponent>();
	//	RB->IsKineimatic = true;
	CameraObject->SetParent(GetOwner());
	const float EyeHeight = 2.0f;
	CameraObject->GetTransform()->SetLocalPosition(glm::vec3(0, EyeHeight, 0));
	CurrnetRot.y = 180.0f;
}
void BleedOutPlayer::TickBleedout()
{
	Mhealth->TakeDamage(BleedOutRate*Engine::GetDeltaTime(), true);
	SecondsLeft = Mhealth->GetCurrentHealth() / BleedOutRate;
}
static ConsoleVariable Sensitivity("sensitivity", 1.0f, ECVarType::ConsoleOnly);
static ConsoleVariable GodMode("god", 1, ECVarType::ConsoleAndLaunch);
void BleedOutPlayer::Update(float delta)
{
#if WITH_EDITOR
	if (EditorWindow::GetInstance()->IsEditorEjected())
	{
		return;
	}
#endif
	TickBleedout();
	CheckForGround();
	UpdateMovement(delta);

	glm::vec2 axis = Input::GetMouseInputAsAxis();
	axis *= Sensitivity.GetFloatValue();
	CurrnetRot.y += -axis.x*LookSensitivty;
	glm::quat YRot = glm::quat(glm::radians(glm::vec3(0, CurrnetRot.y, 0)));
	GetOwner()->SetRotation(YRot);
	CurrnetRot.x += axis.y *LookSensitivty;
	if (CameraObject)
	{
		const float YAxisLock = 80.0f;
		CurrnetRot.x = glm::clamp(CurrnetRot.x, -YAxisLock, YAxisLock);
		glm::quat newrot = glm::quat(glm::radians(glm::vec3(CurrnetRot.x, 0, 0)));
		CameraObject->GetTransform()->SetQrot(newrot);
		if (CameraComponent::GetMainCamera() != nullptr)
		{
			CameraComponent::GetMainCamera()->SetUpAndForward(CameraObject->GetTransform()->GetForward(), CameraObject->GetTransform()->GetUp());
			CameraComponent::GetMainCamera()->SetPos(CameraObject->GetTransform()->GetPos());
		}
	}
	Input::SetCursorState(true, false);
	TickAudio();
	LastFrameGrounded = IsGrounded;
	Mhealth->Damageable = !GodMode.GetBoolValue();
}

void BleedOutPlayer::UpdateMovement(float delta)
{
	glm::vec3 TargetVel = glm::vec3(0, 0, 0);
	glm::vec3 right = glm::vec3(0, 0, 1);
	glm::vec3 fwd = glm::vec3(1, 0, 0);
	float Speed = 1;
	if (!IsGrounded)
	{
		Speed = AirSpeedFactor;
	}

	if (Input::GetKey('a'))
	{
		TargetVel += right * Speed;
	}
	if (Input::GetKey('d'))
	{
		TargetVel -= right * Speed;
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
	glm::vec3 NewVel = (RelativeSpeed.z*GetOwner()->GetTransform()->GetRight()) + (RelativeSpeed.x*GetOwner()->GetTransform()->GetForward());
	NewVel.y = CurrentVel.y;
	if (RB != nullptr)
	{
#if 1
		const glm::vec3 tVel = NewVel /*+ ExtraVel*/;
		glm::vec3 correction = tVel - RB->GetVelocity();
		correction.y = 0.0f;
		RB->GetActor()->AddForce(correction * (IsGrounded ? 100 : 25.0f));
#else
		RB->SetLinearVelocity(NewVel);
#endif
	}
	if (Input::GetKeyDown(KeyCode::SPACE) && IsGrounded)
	{
		RB->GetActor()->AddForce((glm::vec3(0, 1, 0) * 5000));
		AudioEngine::PostEvent("Jump", GetOwner());
	}
	
}

void BleedOutPlayer::TickAudio()
{
	Walk.Tick(GetOwner(), IsGrounded, RB->GetVelocity(), false);
}

void WalkAudio::Tick(GameObject* object,bool IsGrounded,glm::vec3 speed, bool isAI)
{
	CurrnetTime -= Engine::GetDeltaTime();
	if (CurrnetTime > 0.0f || !IsGrounded)
	{
		return;
	}
	float vel = glm::length(glm::vec2(speed.xz));
	if (vel >= 5.0f)
	{
		float pc = vel / 40;
		//Log::LogTextToScreen("PC:" + std::to_string(pc));
		CurrnetTime = glm::clamp(0.35f *1.0f - pc, 0.25f, 1.0f);
		if (isAI)
		{
			AudioEngine::PostEvent("AiStep", object);
		}
		else
		{
			AudioEngine::PostEvent("Step", object);
		}		
	}
}
