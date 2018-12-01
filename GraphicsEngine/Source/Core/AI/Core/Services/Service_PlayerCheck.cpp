#include "Service_PlayerCheck.h"
#include "AI/Core/AIDirector.h"
#include "AI/Core/AISystem.h"
#include "AI/Core/Behaviour/BehaviourTree.h"
#include "AI/Core/Behaviour/BTBlackboard.h"
#include "ServiceBase.h"
#include "Core/Utils/MathUtils.h"


Service_PlayerCheck::Service_PlayerCheck(BTValue* ValueTarget, BTValue* distanceValue)
{
	PlayerValue = ValueTarget;
	DistanceValue = distanceValue;
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
}
