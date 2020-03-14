#pragma once
#include "../RenderNode.h"

class RHICommandList;
class Shader_Pair;
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
	Shader_Pair* Test = nullptr;
};

