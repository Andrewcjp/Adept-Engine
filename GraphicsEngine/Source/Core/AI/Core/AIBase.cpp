
#include "AIBase.h"
#include "Core/GameObject.h"
#include "AI/Core/AIController.h"
#include "AISystem.h"
#include "Behaviour/BehaviourTreeManager.h"
#include "Behaviour/BehaviourTree.h"
#include "Core/Platform/PlatformCore.h"
#include "AnimationController.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Rendering/Core/Mesh.h"

AIBase::AIBase()
{}


AIBase::~AIBase()
{
	SafeDelete(AnimController);
}

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
	if (AnimController != nullptr)
	{
		AnimController->Tick(dt);
	}
}

void AIBase::InitComponent()
{
	Controller = GetOwner()->GetComponent<AIController>();
	SetupBrain();
	MeshRendererComponent* mrc = GetOwner()->GetComponentInChildren<MeshRendererComponent>();
	if (mrc != nullptr)
	{
		AnimController = new AnimationController();
		AnimController->Mesh = mrc->GetMesh()->GetSkeletalMesh();
		AnimController->Owner = this;
	}

}
