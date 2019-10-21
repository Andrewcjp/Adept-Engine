#pragma once
#include "../RenderNode.h"
class ParticleRenderNode :public RenderNode
{
public:
	ParticleRenderNode();
	~ParticleRenderNode();

	virtual void OnExecute() override;
	NameNode("Particle Render");
protected:
	virtual void OnNodeSettingChange() override;
	RHICommandList* RenderList = nullptr;

	virtual void OnSetupNode() override;

};

