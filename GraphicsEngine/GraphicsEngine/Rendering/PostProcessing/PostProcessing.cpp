#include "stdafx.h"
#include "PostProcessing.h"
#include "PP_CompostPass.h"
#include "PP_ColourCorrect.h"
#include "PP_Blur.h"
#include "PP_Bloom.h"
#include "RHI/DeviceContext.h"
#include "RHI/RenderAPIs/D3D12/D3D12TimeManager.h"
#include "../RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "../RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "../RHI/DeviceContext.h"
#include "../RHI/RenderAPIs/D3D12/GPUResource.h"
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

void PostProcessing::ExecPPStack(FrameBuffer* targetbuffer)
{
	/*for (int i = 0; i < Effects.size(); i++)
	{
		Effects[i]->RunPass(list,targetbuffer);
	}*/
	//called to post porcess the final rendered scene
	//ColourCorrect->cmdlist->GetDevice()->GetTimeManager()->StartTimer(ColourCorrect->cmdlist, D3D12TimeManager::eGPUTIMERS::PostProcess);
#if 1
	Blur->RunPass(targetbuffer);
	ColourCorrect->RunPass(targetbuffer);

#else 
	ColourCorrect->RunPass(targetbuffer);
	Bloom->RunPass(targetbuffer);
#endif	

	RHI::GetDeviceContext(0)->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
	
	//ColourCorrect->cmdlist->GetDevice()->GetTimeManager()->StartTimer(ColourCorrect->cmdlist, D3D12TimeManager::eGPUTIMERS::PostProcess);

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
}
void PostProcessing::Resize(FrameBuffer* Target)
{
	//ColourCorrect->InitEffect(Target);
	Blur->InitEffect(Target);
	Bloom->InitEffect(Target);
}

void PostProcessing::MakeReadyForPost(RHICommandList* list, FrameBuffer * buffer)
{
	D3D12FrameBuffer* dBuffer = (D3D12FrameBuffer*)buffer;
	dBuffer->GetResource(0)->SetResourceState(((D3D12CommandList*)list)->GetCommandList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}
void PostProcessing::AddCompostPass(FrameBuffer * buffer)
{
	TestEffct->SetInputFrameBuffer(buffer);
	Needscompost = true;
}

