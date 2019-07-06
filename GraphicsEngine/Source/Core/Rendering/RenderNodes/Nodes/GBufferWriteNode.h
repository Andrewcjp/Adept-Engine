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

	virtual std::string GetName() const override;

private:
	RHICommandList* CommandList = nullptr;
};

