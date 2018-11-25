#pragma once

class BehaviourTree;
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

