#pragma once
#include "Rendering\RenderNodes\RenderNode.h"

class Shader_Pair;
class RHICommandList;
class VRXShadingRateNode : public RenderNode
{
public:
	VRXShadingRateNode();
	virtual ~VRXShadingRateNode();

	void OnExecute() override;
	bool IsNodeSupported(const RenderSettings& settings) override;

protected:
	void OnNodeSettingChange() override;
	RHICommandList* List = nullptr;
	Shader_Pair* Shader = nullptr;
	void OnSetupNode() override;

};

