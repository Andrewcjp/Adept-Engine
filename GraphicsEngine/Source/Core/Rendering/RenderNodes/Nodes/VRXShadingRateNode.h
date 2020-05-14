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
	NameNode("VRX.Shading Rate");
protected:
	void OnNodeSettingChange() override;
	RHICommandList* List = nullptr;
	Shader_Pair* Shader = nullptr;
	void OnSetupNode() override;
	RHIBuffer* Constant = nullptr;
	struct GenerationData
	{
		float GeoThreshold = 0.4f;
		int FullResTheshold = 8 * 8;
		int HalfResTheshold = 8 * 8;
		Bool DebugedgeCount = 0;
		Bool WriteZeroImage = false;
	};
	GenerationData Data;
};

