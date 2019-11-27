#pragma once
#include "../RenderNode.h"

class RHITexture;
class OutputToScreenNode : public RenderNode
{
public:
	OutputToScreenNode();
	~OutputToScreenNode();

	virtual void OnExecute() override;


	NameNode("Output To Screen");
protected:
	virtual void OnNodeSettingChange() override;
	RHICommandList* ScreenWriteList = nullptr;

	virtual void OnSetupNode() override;
	void Test();
	RHITexture* Testtex = nullptr;
};

