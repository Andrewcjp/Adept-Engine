#pragma once
namespace EBTNodeType
{
	enum Type
	{
		Service,
		Selector,
		Sequence,
		Limit
	};
};
namespace EBTNodeReturn
{
	enum Type
	{
		Success,
		Failure,
		Running,
		LIMIT
	};
};
class BehaviourTree;
struct BTValue;
class BehaviourTreeNode
{
public:
	virtual EBTNodeReturn::Type ExecuteNode();
	EBTNodeReturn::Type HandleExecuteNode();
	BehaviourTreeNode* Parent = nullptr;
	std::vector<BehaviourTreeNode*> Children;
	BehaviourTree* ParentTree = nullptr;
	template<class T>
	T* AddChildNode(BehaviourTreeNode* node)
	{
		Children.push_back(node);
		node->ParentTree = ParentTree;
		return (T*)node;
	}
	std::vector<BTValue*> BBValues;
};

class BTServiceNode :public BehaviourTreeNode
{
	virtual EBTNodeReturn::Type ExecuteNode() override;
};

class BTMoveToNode :public BehaviourTreeNode
{
public:
	BTMoveToNode(BTValue* GoalPos);
	virtual EBTNodeReturn::Type ExecuteNode() override;
};