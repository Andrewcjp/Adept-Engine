#pragma once
class BehaviourTreeNode;
class GameObject;
class BTBlackboard;
class BehaviourTree
{
public:
	CORE_API BehaviourTree();
	CORE_API virtual ~BehaviourTree();
	void RunTree();
	BehaviourTreeNode* RootNode = nullptr;
	virtual void SetupTree();//a sub tree should use this to setup in code
	GameObject* Target = nullptr;
	BTBlackboard* Blackboard = nullptr;
};

