#pragma once
#include "..\RenderNode.h"
class LightCullingNode: public RenderNode
{
public:
	LightCullingNode();
	~LightCullingNode();

	virtual void OnExecute() override;
	virtual std::string GetName() const override;
protected:
	virtual void OnNodeSettingChange() override;
	virtual void OnSetupNode() override;
	RHICommandList* List = nullptr;
};

