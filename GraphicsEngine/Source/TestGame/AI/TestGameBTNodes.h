#pragma once
#include "AI/Core/Behaviour/BehaviourTreeNode.h"

class MeleeWeapon;
class B_AIBase;
struct BTValue;

class BTMeleeAttackNode :public BehaviourTreeNode
{
public:
	BTMeleeAttackNode();

	virtual void OnAddedToTree() override;
	virtual std::string GetDebugName() override { return "BTMeleeAttackNode"; }
protected:
	EBTNodeReturn::Type ExecuteNode() override;//todo: fix!
private:
	MeleeWeapon* weapon = nullptr;
};

class BTRifleAttackNode :public BehaviourTreeNode
{
public:
	BTRifleAttackNode(BTValue* obj)
	{
		Value = obj;
	};
	int RoundCount = 4;
	float DelayBetween = 0.016f;

	virtual void OnAddedToTree() override;
	virtual std::string GetDebugName() override { return "BTRifleAttackNode"; }
protected:
	void Run();
	void Reset();
	EBTNodeReturn::Type ExecuteNode() override;//todo: fix!
private:
	float CurrentDelay = 0.0f;
	int RemainingRounds = 0;
	B_AIBase* Controller = nullptr;
	BTValue* Value = nullptr;
};