#include "SpawningPool.h"
#include "AI/Core/AISystem.h"
#include "Source/TestGame/AI/TestGame_Director.h"

SpawningPool::SpawningPool()
{
}


SpawningPool::~SpawningPool()
{
}

void SpawningPool::InitComponent()
{
	
}

void SpawningPool::OnDestroy()
{
	AISystem::GetDirector<TestGame_Director>()->NotifySpawningPoolDestruction();
}