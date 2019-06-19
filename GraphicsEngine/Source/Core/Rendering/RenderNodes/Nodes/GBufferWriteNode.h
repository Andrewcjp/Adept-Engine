#pragma once
#include "../RenderNode.h"
class GBufferWriteNode : public RenderNode
{
public:
	GBufferWriteNode();
	~GBufferWriteNode();
	virtual void OnExecute() override;
	virtual void SetupNode() override;

};

