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
	FrameBuffer* BloomBuffer = nullptr;
private:
	PP_Blur * BlurEffect = nullptr;
	class Shader_Bloom* BloomShader = nullptr;
	RHIUAV* UAV = nullptr;

	class PP_CompostPass* Compost = nullptr;
};

