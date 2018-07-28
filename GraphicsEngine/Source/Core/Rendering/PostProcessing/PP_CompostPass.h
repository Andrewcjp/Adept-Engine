#pragma once
#include "PostProcessEffectBase.h"
class PP_CompostPass: public PostProcessEffectBase
{
public:
	PP_CompostPass();
	~PP_CompostPass();
	virtual void ExecPass(RHICommandList * list, FrameBuffer * InputTexture) override;
	virtual void PostSetUpData() override;
	virtual void PostInitEffect(FrameBuffer* Target) override;
	void SetInputFrameBuffer(FrameBuffer* buffer);
private:
	class Shader_Compost * CurrentShader = nullptr;
	FrameBuffer* InputFramebuffer = nullptr;

};

