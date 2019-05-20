#pragma once
#include "PostProcessEffectBase.h"
class PP_SSAO: public PostProcessEffectBase
{
public:
	PP_SSAO();
	~PP_SSAO();
protected:
	virtual void ExecPass(RHICommandList * list, FrameBuffer * InputTexture) override;
	virtual void PostSetUpData() override;
	virtual void PostInitEffect(FrameBuffer* Target) override;
private:
	FrameBuffer* SSAOOutput;
};

