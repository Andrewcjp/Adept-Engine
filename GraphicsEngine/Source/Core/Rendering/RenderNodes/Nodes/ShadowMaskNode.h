#pragma once
#include "../RenderNode.h"
class ShadowMaskNode : public RenderNode
{
public:
	ShadowMaskNode();
	~ShadowMaskNode();

	void OnExecute() override;
	NameNode("Shadow Mask");
protected:
	void OnNodeSettingChange() override;
	RHICommandList* List = nullptr;
	void OnSetupNode() override;
};

