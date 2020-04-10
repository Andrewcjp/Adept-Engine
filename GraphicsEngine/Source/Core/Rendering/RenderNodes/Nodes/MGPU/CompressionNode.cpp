#include "CompressionNode.h"
#include "../../../Shaders/Shader_Pair.h"
#include "../../StorageNodeFormats.h"
#include "../../../Core/FrameBuffer.h"
#include "RHI/SFRController.h"
#include "../../../Core/Screen.h"
#include "RHI/RHITimeManager.h"

CompressionNode::CompressionNode()
{
	OnNodeSettingChange();
	NodeEngineType = ECommandListType::Compute;
	SetWorkType(ENodeWorkFocus::Compute);
}

CompressionNode::~CompressionNode()
{
}

void CompressionNode::OnExecute()
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
	

	eTEXTURE_FORMAT TargetFormat = RHI::GetRenderSettings()->GetCurrnetSFRSettings().Use8BitCompression ? eTEXTURE_FORMAT::FORMAT_R8_UINT : eTEXTURE_FORMAT::FORMAT_R16_UINT;
	if (Compress)
	{
		DECALRE_SCOPEDGPUCOUNTER(List, "Compress");
		RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(FourCompCompressShader);
		List->SetPipelineStateDesc(Desc);

		RHIBuffer* BufferTarget = GetBufferFromInput(1);
		RHIViewDesc v = RHIViewDesc::DefaultUAV();
		v.Format = TargetFormat;
		List->SetUAV(BufferTarget, FourCompCompressShader->GetSlotForName("OutputBuffer"), v);
		List->SetFrameBufferTexture(Target, "InputData");
		List->SetConstantBufferView(RectData, 0, "AreaData");
		glm::ivec2 Res = glm::ivec2(Width, Target->GetHeight());
		List->SetRootConstant(FourCompCompressShader->GetSlotForName("Resolution"), 1, &Res, 0);
		List->DispatchSized(Width, Target->GetHeight(), 1);
		List->UAVBarrier(BufferTarget);
	}
	else
	{
		DECALRE_SCOPEDGPUCOUNTER(List, "Decompress");
		RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(FourCompDeCompressShader);
		List->SetPipelineStateDesc(Desc);
		RHIBuffer* BufferTarget = GetBufferFromInput(1);
		RHIViewDesc v = RHIViewDesc::DefaultSRV();
		v.Format = TargetFormat;
		List->SetBuffer(BufferTarget, FourCompDeCompressShader->GetSlotForName("CompressedData"), v);
		List->SetUAV(Target, "TargetFrameBuffer");
		List->SetConstantBufferView(RectData, 0, "AreaData");
		glm::ivec2 Res = glm::ivec2(Width, Target->GetHeight());
		List->SetRootConstant(FourCompDeCompressShader->GetSlotForName("Resolution"), 1, &Res, 0);
		List->DispatchSized(Width, Target->GetHeight(), 1);
		List->UAVBarrier(Target);
	}
	if (List->GetDeviceIndex() == 1)
	{
		List->GetDevice()->GetTimeManager()->EndTotalGPUTimer(List);
	}
	SetEndStates(List);
	List->Execute();

}

void CompressionNode::SetCompressMode(bool state)
{
	Inputs.clear();
	Compress = state;
	OnNodeSettingChange();
}

void CompressionNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, Compress ? EResourceState::Non_PixelShader : EResourceState::UAV, StorageFormats::DontCare, "Target RT");
	AddResourceInput(EStorageType::Buffer, Compress ? EResourceState::UAV : EResourceState::Non_PixelShader, StorageFormats::CompressedData, "");
}

void CompressionNode::OnSetupNode()
{
	std::string Use8Bit = RHI::GetRenderSettings()->GetCurrnetSFRSettings().Use8BitCompression ? "1" : "0";
	FourCompCompressShader = new Shader_Pair(Context, { "Compression\\Compress4to3Comp" }, { EShaderType::SHADER_COMPUTE }, { ShaderProgramBase::Shader_Define("USE_8BIT",Use8Bit) });
	FourCompDeCompressShader = new Shader_Pair(Context, { "Compression\\DeCompress4to3Comp" }, { EShaderType::SHADER_COMPUTE }, { ShaderProgramBase::Shader_Define("USE_8BIT",Use8Bit) });

	List = RHI::CreateCommandList(ECommandListType::Compute, Context);
	RectData = RHI::CreateRHIBuffer(ERHIBufferType::Constant, Context);
	RectData->CreateConstantBuffer(sizeof(OffsetXY), 1);
	OffsetXY = glm::ivec2(0, 0);
	RectData->UpdateConstantBuffer(&OffsetXY);
}
