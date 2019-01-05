#include "Service_PlayerCheck.h"
#include "AI/Core/AIDirector.h"
#include "AI/Core/AISystem.h"
#include "AI/Core/Behaviour/BehaviourTree.h"
#include "AI/Core/Behaviour/BTBlackboard.h"
#include "ServiceBase.h"
#include "Core/Utils/MathUtils.h"
#include "Core/Engine.h"
#include "Physics/SimTD/TDPhysicsEngine.h"
#include "Core/Utils/DebugDrawers.h"
#include "Core/Components/RigidbodyComponent.h"


Service_PlayerCheck::Service_PlayerCheck(BTValue* ValueTarget, BTValue* distanceValue, BTValue* RayCheck)
{
	PlayerValue = ValueTarget;
	DistanceValue = distanceValue;
	RayCheckValue = RayCheck;
}

Service_PlayerCheck::~Service_PlayerCheck()
{}

void Service_PlayerCheck::Tick()
{
	GameObject* t = AISystem::GetDirector<AIDirector>()->GetPlayerObject();
	if (t != nullptr)
	{
		PlayerValue->ObjectPtr = t;
	}
	if (DistanceValue != nullptr)
	{
		if (t != nullptr)
		{
			glm::vec3 vec = t->GetPosition() - Owner->AIGameObject->GetPosition();
			DistanceValue->FloatValue = glm::length(vec);
		}
		else
		{
			DistanceValue->FloatValue = MathUtils::FloatMAX;
		}
	}
	if (RayCheckValue != nullptr && t != nullptr)
	{
		const glm::vec3 Offset = glm::vec3(0, 2, 0);
		const glm::vec3 Pos = Owner->AIGameObject->GetPosition() + Offset;
		const glm::vec3 DirToPlayer = glm::normalize(t->GetPosition() - Pos);
		RayHit hit;
		std::vector<RigidBody*> IgnoreActors;
		if (Owner->AIGameObject != nullptr)
		{
			RigidbodyComponent* CMP = Owner->AIGameObject->GetComponent<RigidbodyComponent>();
			if (CMP != nullptr)
			{
				IgnoreActors.push_back(CMP->GetActor());
			}
		}

		if (Engine::GetPhysEngineInstance()->RayCastScene(Pos, DirToPlayer, VisionDistance, &hit, IgnoreActors))
		{
			DebugDrawers::DrawDebugLine(Pos, Pos + DirToPlayer * VisionDistance, (hit.HitObject == t) ? glm::vec3(1) : glm::vec3(1, 0, 0), false, 0.5f);
			//DebugDrawers::DrawDebugSphere(hit.position, 1.0f, glm::vec3(1), 12, false, 0.5);*/
			RayCheckValue->IntValue = (hit.HitObject == t);
		}
		else
		{
			RayCheckValue->IntValue = 0;
		}
	}
}
