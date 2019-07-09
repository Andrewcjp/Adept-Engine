#include "PostProcessing.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Utils/MemoryUtils.h"
#include "PP_Bloom.h"
#include "PP_Blur.h"
#include "PP_ColourCorrect.h"
#include "PP_Debug.h"
#include "PP_SSAO.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHI_inc.h"

PostProcessing* PostProcessing::Instance = nullptr;
void PostProcessing::StartUp()
{
	Instance = new PostProcessing();
}

void PostProcessing::ShutDown()
{}

PostProcessing::PostProcessing()
{
	Instance = this;
}

PostProcessing::~PostProcessing()
{
	Instance = nullptr;
	MemoryUtils::DeleteVector(Effects);
}

void PostProcessing::Update()
{

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

	SSAO = new PP_SSAO();
	SSAO->SetUpData();
	SSAO->InitEffect(Target);
	Debug = new PP_Debug();
	Debug->SetUpData();
	Debug->InitEffect(Target);
	//AddEffect(Debug);

	//AddEffect(SSAO);

	//AddEffect(Bloom);
	//AddEffect(Blur);
	//AddEffect(ColourCorrect);
}
void PostProcessing::Resize(FrameBuffer* Target)
{
	//Bloom->InitEffect(Target);
//	Blur->InitEffect(Target);
	//Bloom->InitEffect(Target);
	//ColourCorrect->AddtiveBuffer = Bloom->BloomBuffer;
}

PostProcessing * PostProcessing::Get()
{
	return Instance;
}

