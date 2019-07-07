#pragma once
#include "..\RenderNode.h"
class UpdateReflectionsNode: public RenderNode
{
public:
	UpdateReflectionsNode();
	~UpdateReflectionsNode();

	virtual void OnExecute() override;

protected:
	virtual void OnNodeSettingChange() override;


	virtual void OnSetupNode() override;

};

