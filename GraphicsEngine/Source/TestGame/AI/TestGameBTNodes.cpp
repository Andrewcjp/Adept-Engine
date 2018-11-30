#include "TestGameBTNodes.h"
#include "AI/Core/Behaviour/BehaviourTree.h"
#include "AI/Core/Behaviour/BehaviourTreeNode.h"
#include "Source/TestGame/Components/MeleeWeapon.h"

BTAttackNode::BTAttackNode()
{}

void BTAttackNode::OnAddedToTree()
{
	weapon = ParentTree->Target->GetComponent<MeleeWeapon>();
}

EBTNodeReturn::Type BTAttackNode::ExecuteNode()
{
	if (weapon != nullptr)
	{
		weapon->Fire();
	}
	return EBTNodeReturn::Success;
}
