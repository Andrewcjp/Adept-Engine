#pragma once
#include "../../RenderNode.h"
class VRLoopNode : public RenderNode
{
public:
	VRLoopNode();
	~VRLoopNode();
	typedef  std::function<RenderNode* (RenderNode* FirstNode)> LoopBodyFunction;
	void SetLoopBody(LoopBodyFunction func);
	void OnExecute() override;
	NameNode("VRLoopNode");

	void OnGraphCreate() override;

private:
	LoopBodyFunction LoopBodyFunctionPtr;
protected:
	void OnSetupNode() override;

};

