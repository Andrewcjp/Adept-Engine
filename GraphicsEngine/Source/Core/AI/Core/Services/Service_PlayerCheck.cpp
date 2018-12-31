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
			glm::vec3 vec = t->GetPosition() - Owner->Target->GetPosition();
			DistanceValue->FloatValue = glm::length(vec);
		}
		else
		{
			DistanceValue->FloatValue = MathUtils::FloatMAX;
		}
	}
	if (RayCheckValue != nullptr)
	{
		const glm::vec3 DirToPlayer = glm::normalize(t->GetPosition() - Owner->Target->GetPosition());
		RayHit hit;
		if (Engine::GetPhysEngineInstance()->RayCastScene(Owner->Target->GetPosition(), DirToPlayer, VisionDistance, &hit))
		{
			DebugDrawers::DrawDebugLine(Owner->Target->GetPosition(), Owner->Target->GetPosition() + DirToPlayer * VisionDistance, (hit.HitObject == t) ? glm::vec3(1) :glm::vec3(1,0,0), false, 0.5f);
			DebugDrawers::DrawDebugSphere(hit.position, 1.0f, glm::vec3(1),12,false,0.5);
			RayCheckValue->IntValue = (hit.HitObject == t);
		}
		else
		{
			RayCheckValue->IntValue = 0;
		}
	}
}
