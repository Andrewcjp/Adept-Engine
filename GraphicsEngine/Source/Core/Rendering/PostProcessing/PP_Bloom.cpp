
#include "PP_Bloom.h"
#include "Rendering/Shaders/PostProcess/Shader_Bloom.h"
#include "RHI/RHI_inc.h"
#include "PP_Blur.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Assets/ShaderCompiler.h"
#include "../Core/FrameBuffer.h"
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
	list->SetUAV(BloomBuffer, 1);
	list->SetFrameBufferTexture(InputTexture, 0);

	list->Dispatch(InputTexture->GetWidth() / ThreadCount, InputTexture->GetHeight() / ThreadCount, 1);
	list->UAVBarrier(InputTexture);
	BlurEffect->ExecPass(list, BloomBuffer);

	desc.ShaderInUse = ShaderCompiler::GetShader<Shader_Bloom_Compost>();
	list->SetPipelineStateDesc(desc);
	list->SetUAV(InputTexture, 1);
	list->SetFrameBufferTexture(BloomBuffer, 0);

	list->Dispatch(InputTexture->GetWidth() / ThreadCount, InputTexture->GetHeight() / ThreadCount, 1);
	list->UAVBarrier(InputTexture);
}

void PP_Bloom::PostSetUpData()
{
	BlurEffect = new PP_Blur();
	BlurEffect->SetUpData();
	BloomShader = ShaderCompiler::GetShader<Shader_Bloom>();
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
	Desc.SimpleStartingState = EResourceState::Non_PixelShader;
	BloomBuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), Desc);
	BlurEffect->InitEffect(BloomBuffer);
}
