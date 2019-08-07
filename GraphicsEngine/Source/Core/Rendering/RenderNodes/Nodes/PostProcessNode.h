#pragma once
#include "../RenderNode.h"
class PostProcessNode : public RenderNode
{
public:
	PostProcessNode();
	~PostProcessNode();

	virtual void OnExecute() override;


	NameNode("Post Process")

protected:
	virtual void OnNodeSettingChange() override;

};

