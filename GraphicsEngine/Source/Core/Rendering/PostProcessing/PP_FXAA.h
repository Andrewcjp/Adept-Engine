#pragma once
#include "Rendering\PostProcessing\PostProcessEffectBase.h"
class PP_FXAA : public PostProcessEffectBase
{
public:
	PP_FXAA();
	~PP_FXAA();
protected:
	void ExecPass(RHICommandList * list, FrameBuffer * InputTexture) override;	
	void PostSetUpData() override;
	void PostInitEffect(FrameBuffer* Target) override;
	FrameBuffer* TempBuffer;
};

