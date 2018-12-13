#pragma once
class BehaviourTreeNode;
class GameObject;
class BTBlackboard;
class BehaviourTree
{
public:
	CORE_API BehaviourTree();
	CORE_API virtual ~BehaviourTree();
	void RunTree(float dt);
	BehaviourTreeNode* RootNode = nullptr;
	virtual void SetupTree();//a sub tree should use this to setup in code
	GameObject* Target = nullptr;
	BTBlackboard* Blackboard = nullptr;
	BehaviourTreeNode* RunningNode = nullptr;
	BehaviourTreeNode* DebugCurrnetNode = nullptr;
private:
	float CoolDownTime = 0.0f;
	float UpdateRate = 0.2f;
};

