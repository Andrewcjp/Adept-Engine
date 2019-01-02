#include "TestGameBTNodes.h"
#include "AI/Core/Behaviour/BehaviourTree.h"
#include "B_AIBase.h"
#include "Source/TestGame/Components/MeleeWeapon.h"
#include "AI/Core/Behaviour/BTBlackboard.h"
#include "AI/Core/AIController.h"
#include "AI/Core/AnimationController.h"
#include "PossessedSoldier/PossessedSoldier.h"

BTMeleeAttackNode::BTMeleeAttackNode()
{}

void BTMeleeAttackNode::OnAddedToTree()
{
	weapon = ParentTree->AIGameObject->GetComponent<MeleeWeapon>();
}

EBTNodeReturn::Type BTMeleeAttackNode::ExecuteNode()
{
	if (weapon != nullptr)
	{
		weapon->Fire();
	}
	if (ParentTree->AIGameObject->GetComponent<AIBase>()->GetAnimController())
	{
		ParentTree->AIGameObject->GetComponent<AIBase>()->GetAnimController()->TriggerAttack(1.5f);
	}
	return EBTNodeReturn::Success;
}

void BTRifleAttackNode::OnAddedToTree()
{
	Controller = ParentTree->AIGameObject->GetComponent<B_AIBase>();
	Reset();
}

void BTRifleAttackNode::Run()
{
	CurrentDelay -= Engine::GetDeltaTime();
	CurrentBurstCoolDown -= Engine::GetDeltaTime();
	if (CurrentDelay > 0.0f || CurrentBurstCoolDown > 0.0f)
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
	AIController* controller = ParentTree->AIGameObject->GetComponent<AIController>();
	controller->SetLookAt(Target->GetPosition());
	controller->LookAtTarget = true;
	PossessedSoldier* Sol = ParentTree->AIGameObject->GetComponent<PossessedSoldier>();
	if (Sol != nullptr)
	{
		Sol->LookAt(Target->GetPosition());
		const glm::vec3 dir = Target->GetPosition()- (ParentTree->AIGameObject->GetPosition()+glm::vec3(0,2,0));
		Controller->MainWeapon->AIForward = glm::normalize(dir);
	}
	
	if (RemainingRounds == 0)
	{
		CurrentBurstCoolDown = CoolDownTime;
	}
}

void BTRifleAttackNode::Reset()
{
	RemainingRounds = RoundCount;
	CurrentDelay = 0.0f;
	CurrentBurstCoolDown = 0.0f;
}

EBTNodeReturn::Type BTRifleAttackNode::ExecuteNode()
{
	Run();
	if (CurrentDelay > 0.0f && RemainingRounds > 0)
	{
		return EBTNodeReturn::Running;
	}
	if (CurrentBurstCoolDown > 0.0f)
	{
		return EBTNodeReturn::Running;
	}
	Reset();
	return EBTNodeReturn::Success;
}
