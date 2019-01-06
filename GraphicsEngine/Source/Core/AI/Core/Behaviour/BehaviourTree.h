#pragma once
/*!  \addtogroup AI 
* @{ */
class BehaviourTreeNode;
class GameObject;
class BTBlackboard;
class AIBase;
class BehaviourTree
{
public:
	CORE_API BehaviourTree();
	CORE_API virtual ~BehaviourTree();
	void RunTree(float dt);
	BehaviourTreeNode* RootNode = nullptr;
	///a sub tree should use this function to setup itself in code (if needed)
	virtual void SetupTree();
	GameObject* AIGameObject = nullptr;
	BTBlackboard* Blackboard = nullptr;
	BehaviourTreeNode* RunningNode = nullptr;
	BehaviourTreeNode* DebugCurrnetNode = nullptr;
	AIBase* AIBasePtr = nullptr;
	bool Active = true;
	/// Returns the Timestep for this tree as it will be different from the Current frame time as BTs are ticked slower
	CORE_API float GetTickDeltaTime();
private:
	float CoolDownTime = 0.0f;
	float UpdateRate = 0.2f;
};

