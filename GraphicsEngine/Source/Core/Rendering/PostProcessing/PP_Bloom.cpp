
#include "PP_Bloom.h"
#include "Rendering/Shaders/PostProcess/Shader_Bloom.h"
#include "RHI/RHI_inc.h"
#include "PP_Blur.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Assets/ShaderComplier.h"
PP_Bloom::PP_Bloom()
{}


PP_Bloom::~PP_Bloom()
{
	SafeDelete(BlurEffect);
	EnqueueSafeRHIRelease(BloomBuffer);
}

void PP_Bloom::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	const int ThreadCount = 8;

	RHIPipeLineStateDesc desc;
	desc.InitOLD(false, false, true);
	desc.ShaderInUse = BloomShader;
	list->SetPipelineStateDesc(desc);
	BloomBuffer->BindUAV(list, 1);
	list->SetFrameBufferTexture(InputTexture, 0);

	list->Dispatch(InputTexture->GetWidth() / ThreadCount, InputTexture->GetHeight() / ThreadCount, 1);
	list->UAVBarrier(InputTexture->GetUAV());
	BlurEffect->ExecPass(list, BloomBuffer);

	desc.ShaderInUse = ShaderComplier::GetShader<Shader_Bloom_Compost>();
	list->SetPipelineStateDesc(desc);
	InputTexture->BindUAV(list, 1);
	list->SetFrameBufferTexture(BloomBuffer, 0);

	list->Dispatch(InputTexture->GetWidth() / ThreadCount, InputTexture->GetHeight() / ThreadCount, 1);
	list->UAVBarrier(InputTexture->GetUAV());
}

void PP_Bloom::PostSetUpData()
{
	BlurEffect = new PP_Blur();
	BlurEffect->SetUpData();
	BloomShader = ShaderComplier::GetShader<Shader_Bloom>();
}

void PP_Bloom::PostInitEffect(FrameBuffer * Target)
{
	if (BloomBuffer != nullptr)
	{
		EnqueueSafeRHIRelease(BloomBuffer);
	}
	RHIFrameBufferDesc Desc = Target->GetDescription();

	Desc.Width = Target->GetWidth();
	Desc.Height = Target->GetHeight();
	Desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	BloomBuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), Desc);
	BlurEffect->InitEffect(BloomBuffer);
}
