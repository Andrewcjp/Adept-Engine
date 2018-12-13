
#include "BehaviourTree.h"
#include "BehaviourTreeNode.h"
#include "BTBlackboard.h"


BehaviourTree::BehaviourTree()
{
	Blackboard = new BTBlackboard();
	RootNode = new BehaviourTreeNode();
	RootNode->ParentTree = this;
}

BehaviourTree::~BehaviourTree()
{}

void BehaviourTree::RunTree(float dt)
{
	if (RootNode == nullptr)
	{
		return;
	}
	CoolDownTime -= dt;
	if (CoolDownTime > 0.0f)
	{
		return;
	}
	CoolDownTime = UpdateRate;
	RootNode->HandleExecuteNode();

	if (DebugCurrnetNode != nullptr && Target != nullptr)
	{
		Log::LogTextToScreen("AI " + Target->GetName() + "Current Node " + DebugCurrnetNode->GetDebugName(), UpdateRate);
	}
}

void BehaviourTree::SetupTree()
{

}
