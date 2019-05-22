#pragma once
#include "PostProcessEffectBase.h"
class PP_Debug:public PostProcessEffectBase
{
public:
	PP_Debug();
	~PP_Debug();
protected:
	virtual void ExecPass(RHICommandList * list, FrameBuffer * InputTexture) override;
	virtual void PostSetUpData() override;
	virtual void PostInitEffect(FrameBuffer* Target) override;

};

