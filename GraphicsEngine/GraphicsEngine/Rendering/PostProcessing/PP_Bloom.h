#pragma once
#include "PostProcessEffectBase.h"
#include "PP_Blur.h"

class PP_Bloom : public PostProcessEffectBase
{
public:
	PP_Bloom();
	~PP_Bloom();

	// Inherited via PostProcessEffectBase
	virtual void ExecPass(RHICommandList * list, FrameBuffer * InputTexture) override;
	virtual void PostSetUpData() override;
	void PostPass() override;
	virtual void PostInitEffect(FrameBuffer * Target) override;
private:
	PP_Blur * BlurEffect = nullptr;
	class Shader_Bloom* BloomShader = nullptr;
	RHIUAV* UAV = nullptr;
	FrameBuffer* BloomBuffer = nullptr;
	class PP_CompostPass* Compost = nullptr;
};

