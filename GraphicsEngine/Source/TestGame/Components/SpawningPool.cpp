#include "SpawningPool.h"
#include "AI/Core/AISystem.h"
#include "Source/TestGame/AI/TestGame_Director.h"
#include "Source/TestGame/TestGameGameMode.h"

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
	if (GetOwnerScene() && !GetOwnerScene()->IsSceneDestorying())
	{
		AISystem::GetDirector<TestGame_Director>()->NotifySpawningPoolDestruction();
	}
}

void SpawningPool::Update(float delta)
{
	TestGameGameMode* t = (TestGameGameMode*)GetOwnerScene()->GetGameMode();
	if (t->GetPlayer() != nullptr)
	{
		const float distancetoplayer = glm::distance(GetOwner()->GetPosition(), t->GetPlayer()->GetPosition());
		if (distancetoplayer < 5)
		{
			if (Input::GetKeyDown('E'))
			{
				GetOwner()->Destory();
			}
		}
	}
}

void SpawningPool::SceneInitComponent()
{
}
