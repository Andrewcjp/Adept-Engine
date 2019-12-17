#include "PP_FXAA.h"
#include "..\Shaders\PostProcess\Shader_FXAA.h"
#include "Core\Assets\ShaderComplier.h"
#include "..\Core\FrameBuffer.h"


PP_FXAA::PP_FXAA()
{}


PP_FXAA::~PP_FXAA()
{}

void PP_FXAA::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	if (RHI::GetRenderSettings()->CurrentAAMode != AAMode::FXAA)
	{
		return;
	}
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_FXAA>());
	list->SetPipelineStateDesc(desc);
	InputTexture->CopyToOtherBuffer(TempBuffer, list);
	InputTexture->SetResourceState(list, EResourceState::UAV);
	TempBuffer->SetResourceState(list, EResourceState::Non_PixelShader);
	list->SetUAV(InputTexture, 1);
	list->SetFrameBufferTexture(TempBuffer, 0);
	glm::ivec2 res = glm::ivec2(TempBuffer->GetWidth(), TempBuffer->GetHeight());
	list->SetRootConstant("Res", 2, &res);
	list->DispatchSized(InputTexture->GetWidth(), InputTexture->GetHeight(), 1);
	list->UAVBarrier(InputTexture);
}

void PP_FXAA::PostSetUpData()
{

}

void PP_FXAA::PostInitEffect(FrameBuffer* Target)
{
	if (TempBuffer != nullptr)
	{
		TempBuffer->AutoResize();
		return;
	}
	EnqueueSafeRHIRelease(TempBuffer);
	RHIFrameBufferDesc desc;
	desc = RHIFrameBufferDesc::CreateColour(1, 1);
	desc.SizeMode = EFrameBufferSizeMode::LinkedToRenderScale;
	desc.AllowUnorderedAccess = true;
	desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	TempBuffer->AutoUpdateSize(desc);
	TempBuffer = RHI::CreateFrameBuffer(RHI::GetDefaultDevice(), desc);

}
