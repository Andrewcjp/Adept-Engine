#pragma once
#include "../RenderNode.h"

class Shader_Deferred;
class Shader_Pair;
class DeferredLightingNode : public RenderNode
{
public:
	DeferredLightingNode();
	~DeferredLightingNode();
	virtual void OnSetupNode() override;
	virtual void OnExecute() override;


	virtual void OnNodeSettingChange() override;
	NameNode("Deferred Lighting");
	bool UseScreenSpaceReflection = false;
private:
	RHICommandList* List = nullptr;
	Shader_Deferred* DeferredShader = nullptr;
	Shader_Pair* StencilWriteShader = nullptr;
};

