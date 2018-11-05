#include "Stdafx.h"
#include "TestBTTree.h"
#include "AI/Core/Behaviour/BehaviourTreeNode.h"
#include "AI/Core/Behaviour/BTBlackboard.h"

TestBTTree::TestBTTree()
{}


TestBTTree::~TestBTTree()
{}

void TestBTTree::SetupTree()
{
	BTValue* posptr = Blackboard->AddValue(EBTBBValueType::Vector);
	posptr->vector = glm::vec3(1, 1, 1);
	RootNode->AddChildNode<BTMoveToNode>(new BTMoveToNode(posptr));
	//RootNode->

}
