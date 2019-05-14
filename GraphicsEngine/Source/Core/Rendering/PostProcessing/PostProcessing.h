#pragma once

#include "PostProcessEffectBase.h"

struct DeviceDependentObjects;
class PostProcessing
{

public:
	static PostProcessing* Instance;
	PostProcessing();
	~PostProcessing();
	std::vector<PostProcessEffectBase*> Effects;
	void AddEffect(PostProcessEffectBase* effect);
	void ExecPPStack(DeviceDependentObjects* Object);
	void Init(FrameBuffer* Target);
	void Resize(FrameBuffer * Target);

	//void AddCompostPass(FrameBuffer* buffer);
	class PP_ColourCorrect* ColourCorrect = nullptr;
	class PP_Blur* Blur = nullptr;
	class PP_Bloom* Bloom = nullptr;
private:
	RHICommandList* CommandList = nullptr;
};
