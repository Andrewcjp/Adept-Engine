#include "Stdafx.h"
#include "BehaviourTreeNode.h"
#include "AI/Core/AISystem.h"
#include "BehaviourTree.h"
#include "Core/GameObject.h"
#include "BTBlackboard.h"

EBTNodeReturn::Type BehaviourTreeNode::ExecuteNode()
{
	return EBTNodeReturn::Success;
}

EBTNodeReturn::Type BehaviourTreeNode::HandleExecuteNode()
{
	EBTNodeReturn::Type returnvalue = ExecuteNode();
	if (returnvalue == EBTNodeReturn::Failure)
	{
		return returnvalue;
	}
	//Children should be sorted for priority
	for (int i = 0; i < Children.size(); i++)
	{
		returnvalue = Children[i]->HandleExecuteNode();
		if (returnvalue == EBTNodeReturn::Failure)
		{
			return returnvalue;
		}
	}
	return returnvalue;
}

EBTNodeReturn::Type BTServiceNode::ExecuteNode()
{
	return EBTNodeReturn::Type();
}

BTMoveToNode::BTMoveToNode(BTValue * GoalPos)
{
	BBValues.push_back(GoalPos);
}

EBTNodeReturn::Type BTMoveToNode::ExecuteNode()
{
	glm::vec3 goal = *BBValues[0]->GetValue<glm::vec3>();
	NavigationPath* path = nullptr;
	if (ParentTree->Target == nullptr)
	{
		return EBTNodeReturn::Failure;
	}
	ENavRequestStatus::Type hr = AISystem::Get()->CalculatePath(goal, ParentTree->Target->GetPosition(), &path);
	return (hr == ENavRequestStatus::Complete) ? EBTNodeReturn::Success : EBTNodeReturn::Failure;
}
