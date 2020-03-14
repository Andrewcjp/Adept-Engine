#pragma once
#include "../../RenderNode.h"
class MultiGPULoopNode : public RenderNode
{
public:
	MultiGPULoopNode();
	~MultiGPULoopNode();
	typedef  std::function<RenderNode* (RenderNode* FirstNode, DeviceContext*)> LoopBodyFunction;
	void SetLoopBody(LoopBodyFunction func);
	void OnExecute() override;
	NameNode("MultiGPULoopNode");

	void OnGraphCreate() override;

private:
	LoopBodyFunction LoopBodyFunctionPtr;
protected:
	void OnSetupNode() override;
};

