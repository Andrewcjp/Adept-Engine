#pragma once
#include "PostProcessEffectBase.h"
class PP_ColourCorrect : public PostProcessEffectBase
{
public:
	PP_ColourCorrect();
	~PP_ColourCorrect();
	virtual void ExecPass(RHICommandList * list, FrameBuffer * InputTexture) override;
	virtual void PostSetUpData() override;
	virtual void PostInitEffect(FrameBuffer* Target) override;
private:
	class Shader_ColourCorrect* CurrentShader = nullptr;
};

