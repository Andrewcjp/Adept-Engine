#include "stdafx.h"
#include "PostProcessing.h"
#include "PP_CompostPass.h"
#include "PP_ColourCorrect.h"
PostProcessing* PostProcessing::Instance = nullptr;
PostProcessing::PostProcessing()
{
	Instance = this;
}


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
	//called to post porcess the final rendered scene
	ColourCorrect->RunPass(ColourCorrect->cmdlist, targetbuffer);
}
void PostProcessing::ExecPPStackFinal(FrameBuffer* targetbuffer)
{
	//called as final pass
	if (Needscompost)
	{
		TestEffct->RunPass(TestEffct->cmdlist, targetbuffer);
	}
}
void PostProcessing::Init()
{
	TestEffct = new PP_CompostPass();
	TestEffct->SetUpData();
	TestEffct->InitEffect();	

	ColourCorrect = new PP_ColourCorrect();
	ColourCorrect->SetUpData();
	ColourCorrect->InitEffect();
}

void PostProcessing::AddCompostPass(FrameBuffer * buffer)
{
	TestEffct->SetInputFrameBuffer(buffer);
	Needscompost = true;
}
