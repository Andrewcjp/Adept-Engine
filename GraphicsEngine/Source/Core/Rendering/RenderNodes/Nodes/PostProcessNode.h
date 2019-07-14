#pragma once
#include "../RenderNode.h"
class PostProcessNode : public RenderNode
{
public:
	PostProcessNode();
	~PostProcessNode();

	virtual void OnExecute() override;


	virtual std::string GetName() const override;

protected:
	virtual void OnNodeSettingChange() override;

};

