#pragma once
#include "../RenderNode.h"
class ParticleSimulateNode : public RenderNode
{
public:
	ParticleSimulateNode();
	~ParticleSimulateNode();

	virtual void OnExecute() override;

};

