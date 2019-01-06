#pragma once
/*!  \addtogroup AI
* @{ */
class BehaviourTree;
///this class Handles Owning and Ticking all Behavior Trees in the scene.
class BehaviourTreeManager
{
	
public:
	BehaviourTreeManager();
	~BehaviourTreeManager();
	void AddTree(BehaviourTree* tree);
	void RemoveTree(BehaviourTree * tree);
	void Reset();
	void Tick(float dt);
private:
	std::vector<BehaviourTree*> trees;
};

