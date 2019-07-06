#pragma once
#include "../RenderNode.h"
class ParticleRenderNode :public RenderNode
{
public:
	ParticleRenderNode();
	~ParticleRenderNode();

	virtual void OnExecute() override;


	virtual std::string GetName() const override;

protected:
	virtual void OnNodeSettingChange() override;

};

