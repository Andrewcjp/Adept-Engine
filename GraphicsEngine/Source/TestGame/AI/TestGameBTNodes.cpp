#include "TestGameBTNodes.h"
#include "AI/Core/Behaviour/BehaviourTree.h"
#include "B_AIBase.h"
#include "Source/TestGame/Components/MeleeWeapon.h"
#include "AI/Core/Behaviour/BTBlackboard.h"
#include "AI/Core/AIController.h"

BTMeleeAttackNode::BTMeleeAttackNode()
{}

void BTMeleeAttackNode::OnAddedToTree()
{
	weapon = ParentTree->Target->GetComponent<MeleeWeapon>();
}

EBTNodeReturn::Type BTMeleeAttackNode::ExecuteNode()
{
	if (weapon != nullptr)
	{
		weapon->Fire();
	}
	return EBTNodeReturn::Success;
}

void BTRifleAttackNode::OnAddedToTree()
{
	Controller = ParentTree->Target->GetComponent<B_AIBase>();
	Reset();
}

void BTRifleAttackNode::Run()
{
	CurrentDelay -= Engine::GetDeltaTime();
	if (CurrentDelay > 0.0f)
	{
		return;
	}
	CurrentDelay = DelayBetween;

	GameObject* Target = Value->GetValue<GameObject>();
	if (Target == nullptr)
	{
		RemainingRounds = 0;
		CurrentDelay = 0.0f;
		return;
	}
	if (Controller->FireAt(Target->GetPosition()))//we could fire this round
	{
		RemainingRounds--;
	}
	AIController* controller = ParentTree->Target->GetComponent<AIController>();
	controller->SetLookAt(Target->GetPosition());
	controller->LookAtTarget = true;
}

void BTRifleAttackNode::Reset()
{
	RemainingRounds = RoundCount;
	CurrentDelay = 0.0f;
}

EBTNodeReturn::Type BTRifleAttackNode::ExecuteNode()
{
	Run();
	if (CurrentDelay > 0.0f && RemainingRounds > 0)
	{
		return EBTNodeReturn::Running;
	}
	Reset();
	return EBTNodeReturn::Success;
}
