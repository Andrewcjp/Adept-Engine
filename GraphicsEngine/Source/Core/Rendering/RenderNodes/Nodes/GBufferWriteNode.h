#pragma once
#include "../RenderNode.h"

class RHICommandList;
class GBufferWriteNode : public RenderNode
{
public:
	GBufferWriteNode();
	~GBufferWriteNode();
	virtual void OnExecute() override;
	virtual void OnSetupNode() override;

	NameNode("GBuffer Write");

private:
	RHICommandList* CommandList = nullptr;
};

