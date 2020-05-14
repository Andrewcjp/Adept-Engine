#pragma once
#include "Rendering/RenderNodes/RenderNode.h"
class SimpleNode :public RenderNode
{
public:
	SimpleNode(std::string name, std::function<void(SimpleNode*)> SetupFunc, std::function<void(RHICommandList* list)> ExecuteFunc);
	~SimpleNode();

	void OnExecute() override;

	std::string GetName() const override;

protected:
	void OnNodeSettingChange() override;
	std::function<void(SimpleNode*)> m_SetupFunc;
	std::function<void(RHICommandList* list)> m_ExecuteFunc;
	std::string m_NodeName = "";

};

