#include "ShadowMaskCompressionNode.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/Screen.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_Pair.h"
#include "RHI/RHICommandList.h"
#include "RHI/SFRController.h"
#include "RHI/RHITimeManager.h"

ShadowMaskCompressionNode::ShadowMaskCompressionNode()
{
	OnNodeSettingChange();
	NodeEngineType = ECommandListType::Compute;
}

ShadowMaskCompressionNode::~ShadowMaskCompressionNode()
{
}

void ShadowMaskCompressionNode::OnExecute()
{
	List->ResetList();
	SetBeginStates(List);
	RHIScissorRect rect = RHIScissorRect();
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	int Width = Target->GetWidth();
	if (RHI::GetRenderSettings()->GetCurrnetSFRSettings().Enabled)
	{
		rect = SFRController::GetScissor(1, Screen::GetScaledRes());
		Width = abs(rect.Left - rect.Right);
		OffsetXY = glm::ivec2(rect.Left, rect.Top);
		RectData->UpdateConstantBuffer(&OffsetXY);
	}
	int Height = Target->GetHeight();
	Width /= 8;
	ETextureFormat TargetFormat = ETextureFormat::R8_UINT;
	if (Compress)
	{
		RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(FourCompCompressShader);
		List->SetPipelineStateDesc(Desc);

		RHIBuffer* BufferTarget = GetBufferFromInput(1);
		RHIViewDesc v = RHIViewDesc::DefaultUAV();
		v.Format = TargetFormat;
		List->SetUAV(BufferTarget, FourCompCompressShader->GetSlotForName("OutputBuffer"), v);
		List->SetFrameBufferTexture(Target, "InputData");
		List->SetConstantBufferView(RectData, 0, "AreaData");
		glm::ivec2 Res = glm::ivec2(Width, Height);
		List->SetRootConstant(FourCompCompressShader->GetSlotForName("Resolution"), 1, &Res, 0);
		List->DispatchSized(Width, Height, 1);
		List->UAVBarrier(BufferTarget);
	}
	else
	{
		RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(FourCompDeCompressShader);
		List->SetPipelineStateDesc(Desc);
		RHIBuffer* BufferTarget = GetBufferFromInput(1);
		RHIViewDesc v = RHIViewDesc::DefaultSRV();
		v.Format = TargetFormat;
		List->SetBuffer(BufferTarget, FourCompDeCompressShader->GetSlotForName("CompressedData"), v);
		List->SetUAV(Target, "TargetFrameBuffer");
		List->SetConstantBufferView(RectData, 0, "AreaData");
		glm::ivec2 Res = glm::ivec2(Width, Height);
		List->SetRootConstant(FourCompDeCompressShader->GetSlotForName("Resolution"), 1, &Res, 0);
		List->DispatchSized(Width, Height, 1);
		List->UAVBarrier(Target);
	}
	if (List->GetDeviceIndex() == 1)
	{
		List->GetDevice()->GetTimeManager()->EndTotalGPUTimer(List);
	}
	SetEndStates(List);
	List->Execute();
}

void ShadowMaskCompressionNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, Compress ? EResourceState::Non_PixelShader : EResourceState::UAV, StorageFormats::DontCare, "Target RT");
	AddResourceInput(EStorageType::Buffer, Compress ? EResourceState::UAV : EResourceState::Non_PixelShader, StorageFormats::CompressedData, "");
}

void ShadowMaskCompressionNode::OnSetupNode()
{
	FourCompCompressShader = new Shader_Pair(Context, { "Compression\\CompressShadowMask" }, { EShaderType::SHADER_COMPUTE });
	FourCompDeCompressShader = new Shader_Pair(Context, { "Compression\\DecompressShadowMask" }, { EShaderType::SHADER_COMPUTE });

	List = RHI::CreateCommandList(ECommandListType::Compute, Context);
	RectData = RHI::CreateRHIBuffer(ERHIBufferType::Constant, Context);
	RectData->CreateConstantBuffer(sizeof(OffsetXY), 1);
	OffsetXY = glm::ivec2(0, 0);
	RectData->UpdateConstantBuffer(&OffsetXY);
}
void ShadowMaskCompressionNode::SetCompressMode(bool state)
{
	Inputs.clear();
	Compress = state;
	OnNodeSettingChange();
}
