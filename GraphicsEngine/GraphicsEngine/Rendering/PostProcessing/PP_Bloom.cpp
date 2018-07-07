#include "Stdafx.h"
#include "PP_Bloom.h"
#include "Rendering/Shaders/PostProcess/Shader_Bloom.h"
#include "RHI/RHICommandList.h"
#include "RHI/DeviceContext.h"
#include "PP_Blur.h"
#include "PP_CompostPass.h"
PP_Bloom::PP_Bloom()
{}


PP_Bloom::~PP_Bloom()
{}

void PP_Bloom::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	UAV->Bind(list, 1);
	list->SetFrameBufferTexture(InputTexture, 0);
	list->GetDevice()->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
	const int ThreadCount = 8;
	list->Dispatch(InputTexture->GetWidth()  / ThreadCount, InputTexture->GetHeight() / ThreadCount, 1);
	list->UAVBarrier(UAV);
}

void PP_Bloom::PostSetUpData()
{
	BlurEffect = new PP_Blur();
	BlurEffect->SetUpData();
	BloomShader = new Shader_Bloom();
	CMDlist = RHI::CreateCommandList(ECommandListType::Compute);
	Compost = new PP_CompostPass();
	Compost->SetUpData();
}

void PP_Bloom::PostPass()
{
	BlurEffect->RunPass(BloomBuffer);
	CMDlist->GetDevice()->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
	//Compost->RunPass(nullptr);
}

void PP_Bloom::PostInitEffect(FrameBuffer * Target)
{
	if (UAV != nullptr)
	{
		delete UAV;
	}
	else
	{
		CMDlist->SetPipelineState(PipeLineState{ false,false,true });
		CMDlist->CreatePipelineState(BloomShader);
	}
	UAV = RHI::CreateUAV(RHI::GetDeviceContext(0));
	if (BloomBuffer != nullptr)
	{
		delete BloomBuffer;
	}
	RHIFrameBufferDesc Desc = Target->GetDescription();
	
	Desc.Width = Target->GetWidth();
	Desc.Height = Target->GetHeight() ;
	Desc.StartingState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	BloomBuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), Desc);
	//BloomBuffer->
	UAV->CreateUAVFromFrameBuffer(BloomBuffer);
	BlurEffect->InitEffect(BloomBuffer);
	Compost->InitEffect(BloomBuffer);
	Compost->SetInputFrameBuffer(BloomBuffer);

}
