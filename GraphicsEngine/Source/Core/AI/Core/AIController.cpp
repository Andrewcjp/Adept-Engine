#include "Stdafx.h"
#include "AIController.h"
#include "Core/GameObject.h"
#include "Core/Components/RigidbodyComponent.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "AI/Core/AISystem.h"
#include "Core/Platform/PlatformCore.h"
AIController::AIController()
{}


AIController::~AIController()
{}

void AIController::MoveTo(glm::vec3 pos)
{
	CurrentTarget.IsValid = true;
	CurrentTarget.StaticPos = pos;
	DebugLineDrawer::instance->AddLine(pos, pos + glm::vec3(0, 10, 0), glm::vec3(0, 1, 1), 100);
	ReplanPath();
}

void AIController::MoveTo(GameObject * target)
{
	CurrentTarget.IsValid = true;
	CurrentTarget.MovingTarget = target;

}

void AIController::InitComponent()
{
	Rigidbody = GetOwner()->GetComponent<RigidbodyComponent>();
}

glm::vec3 ProjectPosition(glm::vec3 pos)
{
	pos.y = 0;
	return pos;
}

void AIController::Update(float dt)
{
	if (CurrentTarget.IsValid && Rigidbody != nullptr && Path != nullptr)
	{
		//Point at the Next path node
		float distance = glm::distance(ProjectPosition(GetOwner()->GetPosition()), Path->Positions[CurrentPathIndex]);
		if (distance <= PathNodeArriveRaduis)
		{
			//we have arrived close enought to this node
			CurrentPathIndex++;
			if (CurrentPathIndex >= Path->Positions.size())
			{
				//path complete!
				Path = nullptr;
			}
		}

		if (Path != nullptr)
		{
			//todo: rotation slower
			glm::vec3 TargetDir = GetOwner()->GetPosition() - Path->Positions[CurrentPathIndex];
			TargetDir.y = 0;
			float angle = glm::degrees(glm::atan(TargetDir.x, TargetDir.z));
			glm::quat rot = glm::angleAxis(angle, glm::vec3(0, 1, 0));
			GetOwner()->GetTransform()->SetQrot(rot);
		}
		glm::vec3 finalvel = glm::vec3(0, 0, 0);
		finalvel = GetOwner()->GetTransform()->GetForward() * Speed;
		finalvel.y = Rigidbody->GetActor()->GetLinearVelocity().y;
		Rigidbody->GetActor()->SetLinearVelocity(finalvel);
	}
	else if (Rigidbody != nullptr)
	{
		Rigidbody->GetActor()->SetLinearVelocity(glm::vec3(0, 0, 0));
	}
}

void AIController::ReplanPath()
{
	if (AISystem::Get()->mesh != nullptr)
	{
		ENavRequestStatus::Type Status = AISystem::Get()->mesh->CalculatePath(GetOwner()->GetPosition(), CurrentTarget.GetTargetPos(), &Path);
		ensure(Status == ENavRequestStatus::Complete);
	}
}

glm::vec3 AIController::Target::GetTargetPos()
{
	if (MovingTarget != nullptr)
	{
		return MovingTarget->GetPosition();
	}
	return StaticPos;
}
