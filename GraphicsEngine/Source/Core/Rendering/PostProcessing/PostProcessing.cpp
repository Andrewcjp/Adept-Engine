#include "PostProcessing.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Utils/MemoryUtils.h"
#include "PP_Bloom.h"
#include "PP_Blur.h"
#include "PP_ColourCorrect.h"
#include "PP_Debug.h"
#include "PP_SSAO.h"

#include "RHI/DeviceContext.h"
#include "RHI/RHI_inc.h"

PostProcessing* PostProcessing::Instance = nullptr;
void PostProcessing::StartUp()
{
	Instance = new PostProcessing();
	Instance->Init(nullptr);
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

void PostProcessing::ExecPPStack(FrameBuffer* Target, RHICommandList* CommandList)
{
	SCOPE_CYCLE_COUNTER_GROUP("PostProcessPass", "Render");
	if (Effects.size() == 0)
	{
		//return;
	}
	//wait for graphics to be done with the target buffer
	

	CommandList->StartTimer(EGPUTIMERS::PostProcess);

	CommandList->ResolveVRXFramebuffer(Target);

	for (int i = 0; i < Effects.size(); i++)
	{
		Effects[i]->RunPass(CommandList, Target);
	}
	if (RHI::IsRenderingVR())
	{
		for (int i = 0; i < Effects.size(); i++)
		{
			//			Effects[i]->RunPass(CommandList, Object->RightEyeFramebuffer);
		}
	}
	CommandList->EndTimer(EGPUTIMERS::PostProcess);

	//tell graphics to wait for us to finish with the target buffer in compute
	
}

void PostProcessing::Init(FrameBuffer* Target)
{
	

	/*ColourCorrect = new PP_ColourCorrect();
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
	SSAO->InitEffect(Target);*/
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

