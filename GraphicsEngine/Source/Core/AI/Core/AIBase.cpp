
#include "AIBase.h"
#include "Core/GameObject.h"
#include "AI/Core/AIController.h"
#include "AISystem.h"
#include "Behaviour/BehaviourTreeManager.h"
#include "Behaviour/BehaviourTree.h"
#include "Core/Platform/PlatformCore.h"

AIBase::AIBase()
{}


AIBase::~AIBase()
{}

void AIBase::OnDestroy()
{
	AISystem::Get()->GetBTManager()->RemoveTree(BTTree);
}

void AIBase::SetupBrain()
{
	ensureMsgf(BTTree != nullptr, "AI missing Behaviour Tree");
	if (BTTree->Target == nullptr)
	{
		BTTree->Target = GetOwner();
	}
	AISystem::Get()->GetBTManager()->AddTree(BTTree);
}

void AIBase::Update(float dt)
{
	if (Player.IsValid())
	{
		DistanceToPlayer = glm::distance(Player->GetPosition(), GetOwner()->GetPosition());
	}
}

void AIBase::InitComponent()
{
	Controller = GetOwner()->GetComponent<AIController>();
	SetupBrain();
}
