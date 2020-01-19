#pragma once
#include "../RenderNode.h"

class Shader_Pair;
class RTFilterNode : public RenderNode
{
public:
	RTFilterNode();
	~RTFilterNode();

	void OnExecute() override;


	void RefreshNode() override;

protected:
	void OnSetupNode() override;
	void ClearAccumlate();

	void OnNodeSettingChange() override;
	Shader_Pair* MergeShader;
	Shader_Pair* TemporalShader;
	bool Clear = true;
	RHIBuffer* DenoiserData;
	struct DenoiserParams
	{
		int Debugmode = 0;
	};
	DenoiserParams Params;

	RHIBuffer* FilterData;
	Shader_Pair* BilateralFilter;
	struct FilterParams
	{
			glm::ivec2 Res;
	};
	FilterParams BFilterParams;
};

