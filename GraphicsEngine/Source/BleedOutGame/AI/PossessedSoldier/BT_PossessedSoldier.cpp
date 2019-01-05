#include "BT_PossessedSoldier.h"
#include "AI/Core/Behaviour/BaseDecorator.h"
#include "AI/Core/Behaviour/BehaviourTreeNode.h"
#include "AI/Core/Behaviour/BTBlackboard.h"
#include "AI/Core/Services/Service_PlayerCheck.h"
#include "Source/BleedOutGame/AI/BleedOutBTNodes.h"
#include "Source/BleedOutGame/BleedOutPCH.h"


BT_PossessedSoldier::BT_PossessedSoldier()
{}


BT_PossessedSoldier::~BT_PossessedSoldier()
{}

void BT_PossessedSoldier::SetupTree()
{
	BTValue* obj = Blackboard->AddValue(EBTBBValueType::Object);
	BTValue* RayCheckValid = Blackboard->AddValue(EBTBBValueType::Integer);
	BTValue* Distance = Blackboard->AddValue(EBTBBValueType::Float);
	RootNode->ReturnOnFailure = false;
	BTSelectorNode* selector = nullptr;
	//Firstly try melee

	//secondly Shoot the Player if Can be seen (RayCheckValid)
	selector = RootNode->AddChildNode<BTSelectorNode>(new BTSelectorNode());
	selector->AddDecorator(new BaseDecorator(Distance, EDecoratorTestType::LessThanEqual, 35));
	selector->AddDecorator(new BaseDecorator(Distance, EDecoratorTestType::GreaterThan, 5));
	selector->AddDecorator(new BaseDecorator(RayCheckValid, EDecoratorTestType::GreaterThanEqual, 1));
	selector->AddService(new Service_PlayerCheck(obj, Distance, RayCheckValid));
	selector->AddChildNode<BTRifleAttackNode>(new BTRifleAttackNode(obj));
	selector->AddChildNode<BTWaitNode>(new BTWaitNode(4.0f));

#if 1
	selector = RootNode->AddChildNode<BTSelectorNode>(new BTSelectorNode());
	selector->AddDecorator(new BaseDecorator(Distance, EDecoratorTestType::LessThanEqual, 5));
	selector->AddChildNode<BTRifleAttackNode>(new BTMeleeAttackNode());
	//selector->AddChildNode<BTWaitNode>(new BTWaitNode(0.3f));
#endif

	//Thirdly  Out of range move to the player
	selector = RootNode->AddChildNode<BTSelectorNode>(new BTSelectorNode());
	selector->AddDecorator(new BaseDecorator(obj, EDecoratorTestType::NotNull));
	selector->AddService(new Service_PlayerCheck(obj));
	selector->AddChildNode<BTMoveToNode>(new BTMoveToNode(obj));
	//selector->AddChildNode<BTWaitNode>(new BTWaitNode(0.2f));


}