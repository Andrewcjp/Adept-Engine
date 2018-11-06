
#include "PostProcessing.h"
#include "PP_CompostPass.h"
#include "PP_ColourCorrect.h"
#include "PP_Blur.h"
#include "PP_Bloom.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHI_inc.h"
#include "RHI/DeviceContext.h"
#include "Core/Utils/MemoryUtils.h"
#include "Core/Performance/PerfManager.h"

PostProcessing* PostProcessing::Instance = nullptr;
PostProcessing::PostProcessing()
{
	Instance = this;
}

PostProcessing::~PostProcessing()
{
	Instance = nullptr;
	MemoryUtils::DeleteVector(Effects);
}

void PostProcessing::AddEffect(PostProcessEffectBase * effect)
{
	Effects.push_back(effect);
}

void PostProcessing::ExecPPStack(FrameBuffer* targetbuffer)
{
	SCOPE_CYCLE_COUNTER("PostProcessPass");
	/*for (int i = 0; i < Effects.size(); i++)
	{
		Effects[i]->RunPass(list,targetbuffer);
	}*/
	//called to post porcess the final rendered scene

#if 0
	{
		SCOPE_CYCLE_COUNTER("Bloom");
		Bloom->RunPass(targetbuffer);
	}

	RHI::GetDeviceContext(0)->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
#endif
	ColourCorrect->RunPass(targetbuffer);

	//RHI::GetDeviceContext(0)->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);

	//ColourCorrect->CMDlist->GetDevice()->GetTimeManager()->StartTimer(ColourCorrect->CMDlist, EGPUTIMERS::PostProcess);

}
void PostProcessing::ExecPPStackFinal(FrameBuffer* targetbuffer)
{
	//called as final pass
	if (Needscompost)
	{
		TestEffct->RunPass(targetbuffer);
	}
}
void PostProcessing::Init(FrameBuffer* Target)
{
	TestEffct = new PP_CompostPass();
	TestEffct->SetUpData();
	TestEffct->InitEffect(Target);

	ColourCorrect = new PP_ColourCorrect();
	ColourCorrect->SetUpData();
	ColourCorrect->InitEffect(Target);

	Blur = new PP_Blur();
	Blur->SetUpData();
	Blur->InitEffect(Target);

	Bloom = new PP_Bloom();
	Bloom->SetUpData();
	Bloom->InitEffect(Target);
	ColourCorrect->AddtiveBuffer = Bloom->BloomBuffer;
	AddEffect(Bloom);
	AddEffect(Blur);
	AddEffect(ColourCorrect);
	AddEffect(TestEffct);

	Bloom->IsFirst = true;
	ColourCorrect->IsLast = true;

}
void PostProcessing::Resize(FrameBuffer* Target)
{
	Blur->InitEffect(Target);
	Bloom->InitEffect(Target);
	ColourCorrect->AddtiveBuffer = Bloom->BloomBuffer;
}

//void PostProcessing::MakeReadyForPost(RHICommandList* list, FrameBuffer * buffer)
//{
//	D3D12FrameBuffer* dBuffer = (D3D12FrameBuffer*)buffer;
//	dBuffer->GetResource(0)->SetResourceState(((D3D12CommandList*)list)->GetCommandList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
//}

void PostProcessing::AddCompostPass(FrameBuffer * buffer)
{
	TestEffct->SetInputFrameBuffer(buffer);
	Needscompost = true;
}

