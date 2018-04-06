#pragma once
#include "../RHI/RHICommandList.h"
#include "../Rendering/Shaders/ShaderOutput.h"
class PostProcessEffectBase
{
public:
	PostProcessEffectBase();
	~PostProcessEffectBase();
	void InitEffect();
	void RunPass(RHICommandList * list, FrameBuffer * InputTexture);
	void SetUpData();
	void TestOutput();
	void RenderScreenQuad(RHICommandList* list);
	RHICommandList* cmdlist = nullptr;
private:
	RHIBuffer * VertexBuffer = nullptr;
	ShaderOutput*  CurrentShader = nullptr;
	
};

