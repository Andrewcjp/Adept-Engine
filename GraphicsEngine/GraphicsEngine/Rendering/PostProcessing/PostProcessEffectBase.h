#pragma once
#include "RHI/RHICommandList.h"
#include "Rendering/Shaders/ShaderOutput.h"
class PostProcessEffectBase
{
public:
	PostProcessEffectBase();
	~PostProcessEffectBase();
	
	void RunPass(RHICommandList * list, FrameBuffer * InputTexture);
	void SetUpData();
	void RenderScreenQuad(RHICommandList* list);
	RHICommandList* cmdlist = nullptr;
	virtual void InitEffect();
protected:

	virtual void ExecPass(RHICommandList * list, FrameBuffer * InputTexture) =0;
	virtual void PostSetUpData() = 0;
	virtual void PostInitEffect() = 0;
private:
	RHIBuffer * VertexBuffer = nullptr;
	
	
};

