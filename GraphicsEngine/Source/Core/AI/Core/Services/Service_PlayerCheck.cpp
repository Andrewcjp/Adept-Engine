#include "Service_PlayerCheck.h"
#include "AI/Core/AIDirector.h"
#include "AI/Core/AISystem.h"
#include "AI/Core/Behaviour/BTBlackboard.h"


Service_PlayerCheck::Service_PlayerCheck(BTValue* ValueTarget)
{
	PlayerValue = ValueTarget;
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
}
