#include "SpawningPool.h"
#include "AI/Core/AISystem.h"
#include "AI/BleedOut_Director.h"
#include "BleedOutGameMode.h"
#include "BleedOutHud.h"

SpawningPool::SpawningPool()
{}


SpawningPool::~SpawningPool()
{}

void SpawningPool::InitComponent()
{

}

void SpawningPool::OnDestroy()
{
	if (GetOwnerScene() && !GetOwnerScene()->IsSceneDestorying())
	{
		AISystem::GetDirector<BleedOut_Director>()->NotifySpawningPoolDestruction();
	}
}

void SpawningPool::Update(float delta)
{

	BleedOutGameMode* t = (BleedOutGameMode*)GetOwnerScene()->GetGameMode();
	if (t->GetPlayer() != nullptr)
	{
		const float distancetoplayer = glm::distance(GetOwner()->GetPosition(), t->GetPlayer()->GetPosition());
		if (distancetoplayer < 5)
		{
			((BleedOutHud*)t->GetCurrentHudInstance())->ShowInteractPrompt(true);
			if (Input::GetKeyDown('E'))
			{
				GetOwner()->Destory();
			}
		}
	}
}

void SpawningPool::SceneInitComponent()
{}
