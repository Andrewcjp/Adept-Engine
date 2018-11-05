#include "Stdafx.h"
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

void BehaviourTree::RunTree()
{
	if (RootNode == nullptr)
	{
		return;
	}
	//if (RunningNode != nullptr)
	//{
	//	EBTNodeReturn::Type res = RunningNode->HandleExecuteNode();
	//	if (res == EBTNodeReturn::Running)
	//	{
	//		return;
	//	}
	//	RunningNode = nullptr;
	//}
	RootNode->HandleExecuteNode();
}

void BehaviourTree::SetupTree()
{
	
}
