
#include "PostProcessing.h"
#include "PP_ColourCorrect.h"
#include "PP_Blur.h"
#include "PP_Bloom.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHI_inc.h"
#include "RHI/DeviceContext.h"
#include "Core/Utils/MemoryUtils.h"
#include "Core/Performance/PerfManager.h"
#include "../Renderers/RenderEngine.h"

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

void PostProcessing::ExecPPStack(DeviceDependentObjects* Object)
{
	SCOPE_CYCLE_COUNTER_GROUP("PostProcessPass", "Render");
	if (Effects.size() == 0)
	{
		return;
	}
	//wait for graphics to be done with the target buffer
	RHI::GetDeviceContext(0)->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
	CommandList->ResetList();
	CommandList->StartTimer(EGPUTIMERS::PostProcess);
	for (int i = 0; i < Effects.size(); i++)
	{
		Effects[i]->RunPass(CommandList, Object->MainFrameBuffer);
	}
	if (RHI::IsRenderingVR())
	{
		for (int i = 0; i < Effects.size(); i++)
		{
			Effects[i]->RunPass(CommandList, Object->RightEyeFramebuffer);
		}
	}
	CommandList->EndTimer(EGPUTIMERS::PostProcess);
	CommandList->Execute();
	//tell graphics to wait for us to finish with the target buffer in compute
	RHI::GetDeviceContext(0)->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
}

void PostProcessing::Init(FrameBuffer* Target)
{
	CommandList = RHI::CreateCommandList(ECommandListType::Compute);

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
	//AddEffect(Blur);
	//AddEffect(ColourCorrect);
}
void PostProcessing::Resize(FrameBuffer* Target)
{
	//Blur->InitEffect(Target);
	//Bloom->InitEffect(Target);
	//ColourCorrect->AddtiveBuffer = Bloom->BloomBuffer;
}

