#pragma once
#include "../RenderNode.h"
class ZPrePassNode: public RenderNode
{
public:
	ZPrePassNode();
	~ZPrePassNode();

	virtual void OnExecute() override;

protected:
	virtual void OnNodeSettingChange() override;


	virtual void OnSetupNode() override;
private:
	RHICommandList* Cmdlist = nullptr;
};

