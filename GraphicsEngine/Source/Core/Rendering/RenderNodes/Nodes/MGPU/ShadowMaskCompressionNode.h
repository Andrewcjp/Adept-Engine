#pragma once
#include "../../RenderNode.h"

class Shader_Pair;
class ShadowMaskCompressionNode : public RenderNode
{
	ShadowMaskCompressionNode();
	~ShadowMaskCompressionNode();

public:
	void OnExecute() override;
	void SetCompressMode(bool state);
protected:
	void OnNodeSettingChange() override;
	void OnSetupNode() override;
	bool Compress = true;
private:
	Shader_Pair* FourCompCompressShader = nullptr;
	Shader_Pair* FourCompDeCompressShader = nullptr;
	RHICommandList* List = nullptr;
	RHIBuffer* RectData = nullptr;
	glm::ivec2 OffsetXY = glm::ivec2();
};

