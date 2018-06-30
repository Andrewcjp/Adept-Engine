#pragma once
#include <vector>
#include "PostProcessEffectBase.h"
class PostProcessing
{

public:
	static PostProcessing* Instance;
	PostProcessing();
	~PostProcessing();
	std::vector<PostProcessEffectBase*> Effects;
	void AddEffect(PostProcessEffectBase* effect);
	void ExecPPStack(FrameBuffer* targetbuffer);
	void ExecPPStackFinal(FrameBuffer * targetbuffer);
	void Init(FrameBuffer* Target);
	void Resize(FrameBuffer * Target);
	void MakeReadyForPost(RHICommandList * list, FrameBuffer * buffer);
	void AddCompostPass(FrameBuffer* buffer);
	class PP_CompostPass* TestEffct = nullptr;
	class PP_ColourCorrect* ColourCorrect = nullptr;
	class PP_Blur* Blur = nullptr;
	class PP_Bloom* Bloom = nullptr;
private:
	bool Needscompost = false;
};

//todo:
//Allow Batching of effects into commandlists 
//need to pipeline state switch - need to add