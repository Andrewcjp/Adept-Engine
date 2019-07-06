#pragma once
#include "../RenderNode.h"
class ParticleSimulateNode : public RenderNode
{
public:
	ParticleSimulateNode();
	~ParticleSimulateNode();
	std::string GetName() const
	{
		return "Particle Simulate";
	}
	virtual void OnExecute() override;

};

