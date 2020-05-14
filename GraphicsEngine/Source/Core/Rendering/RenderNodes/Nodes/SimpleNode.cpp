
#include "SimpleNode.h"
SimpleNode::SimpleNode(std::string name, std::function<void(SimpleNode* N)> SetupFunc, std::function<void(RHICommandList*list)> ExecuteFunc)
{
	m_NodeName = name;
	m_SetupFunc = SetupFunc;
	m_ExecuteFunc = ExecuteFunc;
	OnNodeSettingChange();
}

SimpleNode::~SimpleNode()
{}

void SimpleNode::OnExecute()
{
	RHICommandList* list = GetListAndReset();
	m_ExecuteFunc(list);
	ExecuteList(true);
}

std::string SimpleNode::GetName() const
{
	return m_NodeName;
}

void SimpleNode::OnNodeSettingChange()
{
	m_SetupFunc(this);
}
