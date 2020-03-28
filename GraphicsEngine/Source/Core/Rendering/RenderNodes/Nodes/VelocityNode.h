#pragma once
#include "../RenderNode.h"

class Shader_Pair;
class VelocityNode: public RenderNode
{
public:
	VelocityNode();
	virtual ~VelocityNode();
	void OnExecute() override;
	NameNode("Velocity pass");
protected:
	void OnNodeSettingChange() override;


	void OnSetupNode() override;
	RHICommandList* Cmdlist = nullptr;
	Shader_Pair* FixedVelocityShaders = nullptr;
};

