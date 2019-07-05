#pragma once
#include "../RenderNode.h"
class PreSampleShadowNode : public RenderNode
{
public:
	PreSampleShadowNode();
	~PreSampleShadowNode();

	virtual void OnExecute() override;

protected:
	virtual void OnNodeSettingChange() override;
	virtual void OnSetupNode() override;
private:
	RHICommandList* Commandlist = nullptr;
};

