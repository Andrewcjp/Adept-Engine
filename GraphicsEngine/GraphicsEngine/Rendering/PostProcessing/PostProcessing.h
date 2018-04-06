#pragma once
#include <vector>
#include "PostProcessEffectBase.h"
class PostProcessing
{
public:
	PostProcessing();
	~PostProcessing();
	std::vector<PostProcessEffectBase*> Effects;
	void AddEffect(PostProcessEffectBase* effect);
	void ExecPPStack(FrameBuffer* targetbuffer);
	void Init();
	PostProcessEffectBase* TestEffct = nullptr;
};

//todo:
//Allow Batching of effects into commandlists 
//need to pipeline state switch - need to add