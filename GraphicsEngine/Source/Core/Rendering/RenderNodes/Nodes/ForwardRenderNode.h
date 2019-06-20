#pragma once
#include "../RenderNode.h"
class ForwardRenderNode : public RenderNode
{
public:
	ForwardRenderNode();
	~ForwardRenderNode();

	virtual void OnExecute() override;
	virtual void SetupNode() override;
	virtual std::string GetName() const override;

};

