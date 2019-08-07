#pragma once
#include "../../RenderNode.h"
class BranchNode : public RenderNode
{
public:
	BranchNode();
	~BranchNode();
	virtual void OnExecute() override;
	RenderNode* BranchB = nullptr;
	bool Conditonal = true;
	//#RenderNodes FunctionPTR

	NameNode("Branch");

};

