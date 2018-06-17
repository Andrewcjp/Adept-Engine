#include "stdafx.h"
#include "PostProcessing.h"
#include "PP_CompostPass.h"
#include "PP_ColourCorrect.h"
#include "../RHI/DeviceContext.h"
#include "../RHI/RenderAPIs/D3D12/D3D12TimeManager.h"
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
	//ColourCorrect->cmdlist->GetDevice()->GetTimeManager()->StartTimer(ColourCorrect->cmdlist, D3D12TimeManager::eGPUTIMERS::PostProcess);
	ColourCorrect->RunPass(ColourCorrect->cmdlist, targetbuffer);
	//ColourCorrect->cmdlist->GetDevice()->GetTimeManager()->StartTimer(ColourCorrect->cmdlist, D3D12TimeManager::eGPUTIMERS::PostProcess);
}
void PostProcessing::ExecPPStackFinal(FrameBuffer* targetbuffer)
{
	//called as final pass
	if (Needscompost )
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
