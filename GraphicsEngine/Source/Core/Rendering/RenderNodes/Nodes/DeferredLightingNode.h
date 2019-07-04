#pragma once
#include "../RenderNode.h"

class Shader_Deferred;
class DeferredLightingNode : public RenderNode
{
public:
	DeferredLightingNode();
	~DeferredLightingNode();
	virtual void OnSetupNode() override;
	virtual void OnExecute() override;


	virtual void OnNodeSettingChange() override;
	
private:
	RHICommandList* List = nullptr;
	Shader_Deferred* DeferredShader = nullptr;
};

