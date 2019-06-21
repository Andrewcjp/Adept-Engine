#pragma once
#include "../RenderNode.h"
class DeferredLightingNode : public RenderNode
{
public:
	DeferredLightingNode();
	~DeferredLightingNode();
	virtual void OnSetupNode() override;
	virtual void OnExecute() override;


	virtual void OnNodeSettingChange() override;

};

