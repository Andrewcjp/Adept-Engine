#pragma once
#include "PostProcessEffectBase.h"
class PP_Blur : public PostProcessEffectBase
{
public:
	PP_Blur();
	virtual ~PP_Blur();
	virtual void ExecPass(RHICommandList * list, FrameBuffer * InputTexture) override;
private:
	class Shader_Blur* BlurShader = nullptr;
	class Shader_BlurVert* VertBlurShader = nullptr;
	virtual void PostSetUpData() override;
	virtual void PostInitEffect(FrameBuffer* Target) override;
	FrameBuffer * Cache = nullptr;
};

