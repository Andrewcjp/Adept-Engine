
#include "BehaviourTreeManager.h"
#include "BehaviourTree.h"
#include "AI/test/TestBTTree.h"
#include "Core/Utils/VectorUtils.h"

BehaviourTreeManager::BehaviourTreeManager()
{}

BehaviourTreeManager::~BehaviourTreeManager()
{}

void BehaviourTreeManager::AddTree(BehaviourTree * tree)
{
	tree->SetupTree();
	trees.push_back(tree);
}

void BehaviourTreeManager::RemoveTree(BehaviourTree * tree)
{
	VectorUtils::Remove(trees, tree);
}

void BehaviourTreeManager::Tick(float dt)
{
	for (int i = 0; i < trees.size(); i++)
	{
		trees[i]->RunTree(dt);
	}
}
