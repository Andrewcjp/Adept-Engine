
#include "PP_Bloom.h"
#include "Rendering/Shaders/PostProcess/Shader_Bloom.h"
#include "RHI/RHI_inc.h"
#include "PP_Blur.h"
#include "PP_CompostPass.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Assets/ShaderComplier.h"
PP_Bloom::PP_Bloom()
{}


PP_Bloom::~PP_Bloom()
{
	delete BlurEffect;
	delete Compost;
	EnqueueSafeRHIRelease(BloomBuffer);
	EnqueueSafeRHIRelease(UAV);
}

void PP_Bloom::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	{
		SCOPE_CYCLE_COUNTER("PP_Bloom");
		list->GetDevice()->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
	}
	UAV->Bind(list, 1);
	list->SetFrameBufferTexture(InputTexture, 0);
	const int ThreadCount = 8;
	list->Dispatch(InputTexture->GetWidth() / ThreadCount, InputTexture->GetHeight() / ThreadCount, 1);
	list->UAVBarrier(UAV);
}

void PP_Bloom::PostSetUpData()
{
	BlurEffect = new PP_Blur();
	BlurEffect->SetUpData();
	BloomShader = ShaderComplier::GetShader<Shader_Bloom>();
	CMDlist = RHI::CreateCommandList(ECommandListType::Compute);
	Compost = new PP_CompostPass();
	Compost->SetUpData();
}

void PP_Bloom::PostPass()
{
	SCOPE_CYCLE_COUNTER("PP_Bloom::PostPass");
	//BlurEffect->RunPass(BloomBuffer);
	//CMDlist->GetDevice()->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
	//Compost->RunPass(nullptr);
}

void PP_Bloom::PostInitEffect(FrameBuffer * Target)
{
	if (UAV != nullptr)
	{
		EnqueueSafeRHIRelease(UAV);
	}
	else
	{
		CMDlist->SetPipelineState_OLD(PipeLineState{ false,false,true });
		CMDlist->CreatePipelineState(BloomShader);
	}
	UAV = RHI::CreateUAV(RHI::GetDeviceContext(0));
	if (BloomBuffer != nullptr)
	{
		EnqueueSafeRHIRelease(BloomBuffer);
	}
	RHIFrameBufferDesc Desc = Target->GetDescription();

	Desc.Width = Target->GetWidth();
	Desc.Height = Target->GetHeight();
	Desc.StartingState = GPU_RESOURCE_STATES::D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	BloomBuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), Desc);
	//BloomBuffer->

	UAV->CreateUAVFromFrameBuffer(BloomBuffer);
	BlurEffect->InitEffect(BloomBuffer);
	Compost->InitEffect(BloomBuffer);
	Compost->SetInputFrameBuffer(BloomBuffer);

}
