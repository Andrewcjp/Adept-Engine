#include "AIController.h"
#include "AISystem.h"
#include "Core/Components/RigidbodyComponent.h"
#include "Core/GameObject.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Rendering/Core/DebugLineDrawer.h"
static ConsoleVariable FreezeAIOption("freezeai", 0, ECVarType::ConsoleAndLaunch);
AIController::AIController()
{
	FreezeAIOption.SetValue(0);
}


AIController::~AIController()
{}

void AIController::MoveTo(glm::vec3 pos)
{
	CurrentTarget.IsValid = true;
	CurrentTarget.MovingTarget = nullptr;
	CurrentTarget.StaticPos = pos;
	DebugLineDrawer::Get()->AddLine(pos, pos + glm::vec3(0, 10, 0), glm::vec3(0, 1, 1), 100);
	ReplanPath();
}

void AIController::MoveTo(GameObject * target)
{
	CurrentTarget.IsValid = true;
	CurrentTarget.MovingTarget = target;
	ReplanPath();
}

void AIController::SetLookAt(glm::vec3 pos)
{
	CurrentTarget.IsValid = true;
	CurrentTarget.StaticPos = pos;
}

void AIController::InitComponent()
{
	Rigidbody = GetOwner()->GetComponent<RigidbodyComponent>();
	Rigidbody->IsKineimatic = true;
}

glm::vec3 ProjectPosition(glm::vec3 pos)
{
	pos.y = 0;
	return pos;
}

void AIController::Update(float dt)
{
	if (FreezeAIOption.GetBoolValue())
	{
		return;
	}
	if (!Active)
	{
		return;
	}
	if (CurrentTarget.IsValid && RHI::GetFrameCount() % 60 == 0)
	{
		//ReplanPath();
	}
	if (CurrentTarget.IsValid && Rigidbody != nullptr && !Path.PathComplete && Path.PathReady && Path.Positions.size() > 0)
	{
		//Point at the Next path node
		CurrentPathIndex = glm::clamp(CurrentPathIndex, 0, (int)Path.Positions.size() - 1);
		float distance = glm::distance(ProjectPosition(GetOwner()->GetPosition()), ProjectPosition(Path.Positions[CurrentPathIndex]));
		if (distance <= PathNodeArriveRaduis)
		{
			//we have arrived close enough to this node
			CurrentPathIndex++;
			if (CurrentPathIndex >= Path.Positions.size())
			{
				//path complete!
				Path.EndPath();
				return;
			}
		}
		glm::vec3 DirToNode = glm::normalize(Path.Positions[CurrentPathIndex] - GetOwner()->GetPosition());;

		glm::vec3 finalvel = glm::vec3(0, 0, 0);
		finalvel = GetOwner()->GetTransform()->GetForward() * Speed;
		if (true)
		{
			finalvel = DirToNode * Speed;
		}
		finalvel.y = Rigidbody->GetActor()->GetLinearVelocity().y;
		Rigidbody->GetActor()->SetLinearVelocity(finalvel);
	}
	else if (Rigidbody != nullptr)
	{
		Rigidbody->GetActor()->SetLinearVelocity(glm::vec3(0, Rigidbody->GetActor()->GetLinearVelocity().y, 0));
	}

	glm::vec3 DirToTarget = glm::vec3(0, 0, 0);
	if (LookAtTarget)
	{
		DirToTarget = glm::normalize(CurrentTarget.GetTargetPos() - GetOwner()->GetPosition());
	}
	else if (Path.PathReady && !Path.PathComplete && Path.Positions.size() > 0)
	{
		glm::vec3 dir = ProjectPosition(Path.Positions[CurrentPathIndex]) - ProjectPosition(GetOwner()->GetPosition());
		if (glm::length(dir) != 0.0f)
		{
			DirToTarget = glm::normalize(dir);
		}
		else
		{
			DirToTarget = glm::vec3(1, 0, 0);
		}
		CheckNAN(DirToTarget);
	}
	else
	{
		if (CurrentTarget.IsValid)
		{
			DirToTarget = glm::normalize(CurrentTarget.GetTargetPos() - GetOwner()->GetPosition());
		}
	}
	float angle = glm::degrees(glm::atan(DirToTarget.x, DirToTarget.z));
	if (DirToTarget.length() == 0.0f)
	{
		angle = LastAngle;
	}
	else
	{
		angle = glm::mix(angle, LastAngle, TurnRatio);
		LastAngle = angle;
	}
	glm::quat rot = glm::angleAxis(glm::radians(angle), glm::vec3(0, 1, 0));
	GetOwner()->GetTransform()->SetQrot(rot);
}

void AIController::ReplanPath()
{
	if (AISystem::Get()->mesh != nullptr)
	{
		AISystem::Get()->CalculatePath(GetOwner()->GetPosition(), CurrentTarget.GetTargetPos(), &Path);
	}
}

glm::vec3 AIController::Target::GetTargetPos()
{
	if (MovingTarget.IsValid())
	{
		return MovingTarget->GetPosition();
	}
	return StaticPos;
}
