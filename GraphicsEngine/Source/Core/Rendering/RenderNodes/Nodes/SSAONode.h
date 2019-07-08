#pragma once
#include "..\RenderNode.h"
class SSAONode : public RenderNode
{
public:
	SSAONode();
	~SSAONode();

	virtual void OnExecute() override;

protected:
	virtual void OnNodeSettingChange() override;

	virtual void OnSetupNode() override;

private:
	RHICommandList* list = nullptr;
};

