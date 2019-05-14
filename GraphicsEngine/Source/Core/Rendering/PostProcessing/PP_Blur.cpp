
#include "PP_Blur.h"
#include "Rendering/Shaders/PostProcess/Shader_Blur.h"
#include "RHI/RHI_inc.h"
const int ThreadCount = 32;
PP_Blur::PP_Blur()
{}

PP_Blur::~PP_Blur()
{

}

void PP_Blur::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	RHIPipeLineStateDesc desc;
	desc.InitOLD(false, false, true);
	desc.ShaderInUse = BlurShader;
	list->SetPipelineStateDesc(desc);
	InputTexture->BindUAV(list, 1);
	//UAV->Bind(list, 1);
	list->SetFrameBufferTexture(InputTexture, 0);

	list->SetConstantBufferView(VertBlurShader->Blurweights, 0, 2);
	list->Dispatch(InputTexture->GetWidth() / ThreadCount, InputTexture->GetHeight(), 1);
	list->UAVBarrier(InputTexture->GetUAV());
	Cache = InputTexture;
	desc.ShaderInUse = VertBlurShader;
	list->SetPipelineStateDesc(desc);
	InputTexture->BindUAV(list, 1);
	list->SetFrameBufferTexture(Cache, 0);
	list->SetConstantBufferView(VertBlurShader->Blurweights, 0, 2);
	list->Dispatch(Cache->GetWidth(), Cache->GetHeight() / ThreadCount, 1);
	list->UAVBarrier(InputTexture->GetUAV());
}

void PP_Blur::PostSetUpData()
{
	BlurShader = ShaderComplier::GetShader<Shader_Blur>();
	VertBlurShader = ShaderComplier::GetShader<Shader_BlurVert>();
}

void PP_Blur::PostInitEffect(FrameBuffer* Target)
{

}


