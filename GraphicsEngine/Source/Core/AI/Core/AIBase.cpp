
#include "AIBase.h"
#include "Core/GameObject.h"
#include "AIController.h"
#include "AISystem.h"
#include "AnimationController.h"
#include "Behaviour/BehaviourTree.h"
#include "Behaviour/BehaviourTreeManager.h"
#include "Core/Components/MeshRendererComponent.h"
#include "Core/Platform/PlatformCore.h"
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

void AIBase::SetDead()
{
	SetBrainEnabled(false);
	if (AnimController != nullptr)
	{
		AnimController->SetState(EGenericAnimtionStates::Dead);
	}
	IsDying = true;
	DeathTimer = DeathLength;
}

void AIBase::SetBrainEnabled(bool state)
{
	Active = state;
	BTTree->Active = state;
	Controller->Active = state;
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
	if (AnimController != nullptr)
	{
		AnimController->Tick(dt);
	}
	if (IsDying)
	{
		DeathTimer -= dt;
		if (DeathTimer <= 0.0f)
		{
			GetOwner()->Destory();
		}
	}
	if (!Active)
	{
		return;
	}
	if (Player.IsValid())
	{
		DistanceToPlayer = glm::distance(Player->GetPosition(), GetOwner()->GetPosition());
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

AnimationController * AIBase::CreateAnimationController()
{
	return new AnimationController();
}
