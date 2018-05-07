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
	void Init();
	void AddCompostPass(FrameBuffer* buffer);
	class PP_CompostPass* TestEffct = nullptr;
	class PP_ColourCorrect* ColourCorrect = nullptr;
private:
	bool Needscompost = false;
};

//todo:
//Allow Batching of effects into commandlists 
//need to pipeline state switch - need to add