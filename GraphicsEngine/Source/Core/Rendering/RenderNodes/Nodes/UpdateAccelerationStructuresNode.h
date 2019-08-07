#pragma once
#include "..\RenderNode.h"
class UpdateAccelerationStructuresNode : public RenderNode
{
public:
	UpdateAccelerationStructuresNode();
	~UpdateAccelerationStructuresNode();

	virtual void OnExecute() override;


	NameNode("Update Acceleration Structures");

protected:
	virtual void OnNodeSettingChange() override;

};

