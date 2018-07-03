#pragma once
#include "RHI/RHICommandList.h"

class PostProcessEffectBase
{
public:
	PostProcessEffectBase();
	~PostProcessEffectBase();
	
	void RunPass( FrameBuffer * InputTexture);
	void SetUpData();
	void RenderScreenQuad(RHICommandList* list);
	virtual void InitEffect(FrameBuffer* Target);
protected:
	virtual void ExecPass(RHICommandList * list, FrameBuffer * InputTexture) =0;
	virtual void PostSetUpData() = 0;
	virtual void PostInitEffect(FrameBuffer* Target) = 0;
	RHICommandList* CMDlist = nullptr;
	virtual void PostPass() {};
private:
	RHIBuffer * VertexBuffer = nullptr;	
};

