#pragma once
#include "..\RenderNode.h"
class UpdateAccelerationStructuresNode : public RenderNode
{
public:
	UpdateAccelerationStructuresNode();
	~UpdateAccelerationStructuresNode();

	virtual void OnExecute() override;


	virtual std::string GetName() const override;

protected:
	virtual void OnNodeSettingChange() override;

};

