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
class ServiceBase;
class BaseDecorator;
class BehaviourTreeNode
{
public:
	CORE_API BehaviourTreeNode() {}
	CORE_API virtual ~BehaviourTreeNode() {}
	CORE_API virtual EBTNodeReturn::Type HandleExecuteNode();
	CORE_API virtual void OnAddedToTree();
	CORE_API virtual std::string GetDebugName();
	BehaviourTreeNode* Parent = nullptr;
	std::vector<BehaviourTreeNode*> Children;
	BehaviourTree* ParentTree = nullptr;
	template<class T>
	T* AddChildNode(BehaviourTreeNode* node)
	{
		Children.push_back(node);
		node->ParentTree = ParentTree;
		node->OnAddedToTree();
		return (T*)node;
	}
	std::vector<BTValue*> BBValues;
	bool ReturnOnFailure = true;
protected:
	BehaviourTreeNode* RunningChild = nullptr;
	virtual EBTNodeReturn::Type ExecuteNode();
	bool ExecuteChilds = true;
};

class BTMoveToNode :public BehaviourTreeNode
{
public:
	CORE_API BTMoveToNode(BTValue* GoalPos);
	virtual EBTNodeReturn::Type ExecuteNode() override;
	CORE_API virtual std::string GetDebugName() override { return "BTMoveToNode"; };
};
class BTSelectorNode :public BehaviourTreeNode
{
public:
	CORE_API BTSelectorNode() { ExecuteChilds = false; }
	virtual EBTNodeReturn::Type HandleExecuteNode() override;
	virtual EBTNodeReturn::Type ExecuteNode() override;
	CORE_API void AddDecorator(BaseDecorator* dec);
	CORE_API void AddService(ServiceBase* Service);

	CORE_API virtual std::string GetDebugName() override;

private:
	std::vector<BaseDecorator*> Decorators;//Conditionals for this selector
	std::vector<ServiceBase*> Services;//services update value for decorators to check
	bool ContinueUntilFail = true;
};

class BTWaitNode :public BehaviourTreeNode
{
public:
	BTWaitNode(float Time)
	{
		TargetTime = Time;
		Remaining = TargetTime;
	}
	CORE_API virtual std::string GetDebugName() override { return "BTWaitNode"; };
protected:
	CORE_API EBTNodeReturn::Type ExecuteNode() override;//todo: fix!
private:
	float Remaining = 0.0f;
	float TargetTime = 0.0f;
};
