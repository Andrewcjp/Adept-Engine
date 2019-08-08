#pragma once
#include "../RenderNode.h"

class ShadowRenderer;
class ShadowUpdateNode : public RenderNode
{
public:
	ShadowUpdateNode();
	~ShadowUpdateNode();

	virtual void OnExecute() override;


	NameNode("Shadow Update");
	
protected:
	virtual void OnNodeSettingChange() override;
	virtual void OnSetupNode() override;
private:
	ShadowRenderer* mShadowRenderer = nullptr;
	bool Once = true;
	RHICommandList* ShadowList = nullptr;
};

