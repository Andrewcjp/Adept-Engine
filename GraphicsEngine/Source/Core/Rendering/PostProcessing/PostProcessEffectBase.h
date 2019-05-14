#pragma once
#include "RHI/RHICommandList.h"

class PostProcessEffectBase
{
public:
	PostProcessEffectBase();
	virtual ~PostProcessEffectBase();

	void RunPass(RHICommandList* cmdlist, FrameBuffer * InputTexture);

	void SetUpData();
	void RenderScreenQuad(RHICommandList* list);
	void InitEffect(FrameBuffer* Target);
	virtual void Resize(int width, int height);
protected:
	virtual void ExecPass(RHICommandList * list, FrameBuffer * InputTexture) = 0;
	virtual void PostSetUpData() = 0;
	virtual void PostInitEffect(FrameBuffer* Target) = 0;
private:
	RHIBuffer * VertexBuffer = nullptr;
	friend class PostProcessing;
};

