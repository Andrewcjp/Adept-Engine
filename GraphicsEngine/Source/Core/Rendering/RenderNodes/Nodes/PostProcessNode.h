#pragma once
#include "../RenderNode.h"
class PostProcessNode : public RenderNode
{
public:
	PostProcessNode();
	~PostProcessNode();

	virtual void OnExecute() override;



	void OnResourceResize() override;

	NameNode("Post Processing")

protected:
	virtual void OnNodeSettingChange() override;
	RHICommandList* CommandList = nullptr;

	virtual void OnSetupNode() override;

};

