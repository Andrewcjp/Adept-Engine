#pragma once
#include "AI/Core/Behaviour/BehaviourTreeNode.h"

class MeleeWeapon;

class BTAttackNode :public BehaviourTreeNode
{
public:
	BTAttackNode();

	virtual void OnAddedToTree() override;

protected:
	EBTNodeReturn::Type ExecuteNode() override;//todo: fix!
private:
	MeleeWeapon* weapon = nullptr;
};