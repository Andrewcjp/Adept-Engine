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
	RootNode->HandleExecuteNode();
}

void BehaviourTree::SetupTree()
{
	
}
