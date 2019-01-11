
#include "PP_Blur.h"
#include "Rendering/Shaders/PostProcess/Shader_Blur.h"
#include "RHI/RHI_inc.h"
const int ThreadCount = 32;
PP_Blur::PP_Blur()
{}

PP_Blur::~PP_Blur()
{
	EnqueueSafeRHIRelease(VertcmdList);
	EnqueueSafeRHIRelease(UAV);
}

void PP_Blur::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	UAV->Bind(list, 1);
	list->SetFrameBufferTexture(InputTexture, 0);

	list->SetConstantBufferView(VertBlurShader->Blurweights, 0, 2);
	list->Dispatch(InputTexture->GetWidth() / ThreadCount, InputTexture->GetHeight(), 1);
	list->UAVBarrier(UAV);
	Cache = InputTexture;
}

void PP_Blur::PostSetUpData()
{
	BlurShader = ShaderComplier::GetShader<Shader_Blur>();
	VertBlurShader = ShaderComplier::GetShader<Shader_BlurVert>();
	CMDlist = RHI::CreateCommandList(ECommandListType::Compute);
	VertcmdList = RHI::CreateCommandList(ECommandListType::Compute);
}

void PP_Blur::PostPass()
{
	VertcmdList->ResetList();
	RHICommandList* list = VertcmdList;
	UAV->Bind(VertcmdList, 1);
	VertcmdList->SetFrameBufferTexture(Cache, 0);
	VertcmdList->SetConstantBufferView(VertBlurShader->Blurweights, 0, 2);


	list->Dispatch(Cache->GetWidth(), Cache->GetHeight() / ThreadCount, 1);
	list->UAVBarrier(UAV);
	VertcmdList->Execute();
}
void PP_Blur::PostInitEffect(FrameBuffer* Target)
{
	if (UAV != nullptr)
	{
		EnqueueSafeRHIRelease(UAV);
	}
	else
	{
		RHIPipeLineStateDesc desc;
		desc.InitOLD(false, false, true);
		desc.ShaderInUse = BlurShader;
		CMDlist->SetPipelineStateDesc(desc);
		desc.ShaderInUse = VertBlurShader;
		VertcmdList->SetPipelineStateDesc(desc);
	}
	UAV = RHI::CreateUAV(RHI::GetDeviceContext(0));
	UAV->CreateUAVFromFrameBuffer(Target);
}


