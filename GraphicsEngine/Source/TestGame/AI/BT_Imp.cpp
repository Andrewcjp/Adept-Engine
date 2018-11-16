#pragma once
#include "BT_Imp.h"
#include "AI/Core/Behaviour/BehaviourTreeNode.h"
#include "AI/Core/Behaviour/BTBlackboard.h"
#include "AI/Core/Behaviour/BaseDecorator.h"
#include "AI/Core/Services/Service_PlayerCheck.h"

BT_Imp::BT_Imp()
{}


BT_Imp::~BT_Imp()
{}

void BT_Imp::SetupTree()
{
	BTValue* posptr = Blackboard->AddValue(EBTBBValueType::Vector);
	posptr->Vector = glm::vec3(1, 1, 1);
	BTValue* obj = Blackboard->AddValue(EBTBBValueType::Object);
	BTSelectorNode* selector = RootNode->AddChildNode<BTSelectorNode>(new BTSelectorNode());
	selector->Decorators.push_back(new BaseDecorator(obj, EDecoratorTestType::NotNull));
	selector->Services.push_back(new Service_PlayerCheck(obj));
	selector->AddChildNode<BTWaitNode>(new BTWaitNode(0.5f));
	selector->AddChildNode<BTMoveToNode>(new BTMoveToNode(obj));
}
 