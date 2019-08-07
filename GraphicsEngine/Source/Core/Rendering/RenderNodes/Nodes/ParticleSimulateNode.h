#pragma once
#include "../RenderNode.h"
class ParticleSimulateNode : public RenderNode
{
public:
	ParticleSimulateNode();
	~ParticleSimulateNode();
	NameNode("Particle Simulate");
	virtual void OnExecute() override;

};

