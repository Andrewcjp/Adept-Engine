
#include "BehaviourTree.h"
#include "BehaviourTreeNode.h"
#include "BTBlackboard.h"
#include "AI/Core/AISystem.h"

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
	if (!Active)
	{
		return;
	}
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

	if (DebugCurrnetNode != nullptr && AIGameObject != nullptr && (AISystem::GetDebugMode() == EAIDebugMode::BT || AISystem::GetDebugMode() == EAIDebugMode::All))
	{
		Log::LogTextToScreen("AI " + AIGameObject->GetName() + "Current Node " + DebugCurrnetNode->GetDebugName(), UpdateRate);
	}
}

void BehaviourTree::SetupTree()
{
	AIBasePtr = AIGameObject->GetComponent<AIBase>();
}

float BehaviourTree::GetTickDeltaTime()
{
	return UpdateRate;
}
