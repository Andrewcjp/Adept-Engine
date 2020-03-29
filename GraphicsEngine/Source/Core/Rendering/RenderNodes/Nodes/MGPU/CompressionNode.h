#pragma once
#include "../../RenderNode.h"

class Shader_Pair;
class CompressionNode : public RenderNode
{
public:
	CompressionNode();
	virtual ~CompressionNode();
	NameNode("Compression Node");
	void OnExecute() override;
	void SetCompressMode(bool state);
private:
	Shader_Pair* FourCompCompressShader = nullptr;
	Shader_Pair* FourCompDeCompressShader = nullptr;
protected:
	bool Compress = true;
	void OnNodeSettingChange() override;
	void OnSetupNode() override;
	RHICommandList* List = nullptr;
	RHIBuffer* RectData = nullptr;
	glm::ivec2 OffsetXY = glm::ivec2();
};

