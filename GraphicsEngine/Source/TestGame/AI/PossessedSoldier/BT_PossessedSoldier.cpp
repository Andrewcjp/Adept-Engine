#include "BT_PossessedSoldier.h"
#include "AI/Core/Behaviour/BaseDecorator.h"
#include "AI/Core/Behaviour/BehaviourTreeNode.h"
#include "AI/Core/Behaviour/BTBlackboard.h"
#include "AI/Core/Services/Service_PlayerCheck.h"
#include "Source/TestGame/AI/TestGameBTNodes.h"
#include "Source/TestGame/TestGamePCH.h"


BT_PossessedSoldier::BT_PossessedSoldier()
{}


BT_PossessedSoldier::~BT_PossessedSoldier()
{}

void BT_PossessedSoldier::SetupTree()
{
	BTValue* posptr = Blackboard->AddValue(EBTBBValueType::Vector);
	posptr->Vector = glm::vec3(1, 1, 1);
	BTValue* obj = Blackboard->AddValue(EBTBBValueType::Object);
	BTValue* Distance = Blackboard->AddValue(EBTBBValueType::Float);
	RootNode->ReturnOnFailure = false;
	BTSelectorNode* selector = RootNode->AddChildNode<BTSelectorNode>(new BTSelectorNode());
	selector->AddDecorator(new BaseDecorator(Distance, EDecoratorTestType::LessThanEqual, 50));
	selector->AddService(new Service_PlayerCheck(obj, Distance));
	selector->AddChildNode<BTWaitNode>(new BTRifleAttackNode(obj));
	selector->AddChildNode<BTWaitNode>(new BTWaitNode(0.4f));
	//sub node
	selector = RootNode->AddChildNode<BTSelectorNode>(new BTSelectorNode());
	selector->AddDecorator(new BaseDecorator(obj, EDecoratorTestType::NotNull));
	selector->AddService(new Service_PlayerCheck(obj));
	selector->AddChildNode<BTMoveToNode>(new BTMoveToNode(obj));
	//selector->AddChildNode<BTWaitNode>(new BTWaitNode(0.2f));


}