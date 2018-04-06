#include "stdafx.h"
#include "PostProcessing.h"


PostProcessing::PostProcessing()
{}


PostProcessing::~PostProcessing()
{}

void PostProcessing::AddEffect(PostProcessEffectBase * effect)
{
	Effects.push_back(effect);
}

void PostProcessing::ExecPPStack( FrameBuffer* targetbuffer)
{
	/*for (int i = 0; i < Effects.size(); i++)
	{
		Effects[i]->RunPass(list,targetbuffer);
	}*/
	TestEffct->RunPass(TestEffct->cmdlist, targetbuffer);
	
}

void PostProcessing::Init()
{
	TestEffct = new PostProcessEffectBase();
	TestEffct->SetUpData();
	TestEffct->InitEffect();
	
}
