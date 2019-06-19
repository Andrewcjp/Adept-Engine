#pragma once
#include "../RenderNode.h"
class DeferredLightingNode : public RenderNode
{
public:
	DeferredLightingNode();
	~DeferredLightingNode();

	virtual void SetupNode() override;


	virtual void OnExecute() override;

};

