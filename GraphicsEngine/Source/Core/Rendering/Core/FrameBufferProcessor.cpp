#include "Stdafx.h"
#include "FrameBufferProcessor.h"
#include "RHI\RHICommandList.h"
#include "FrameBuffer.h"
#include "Core\Assets\ShaderComplier.h"
#include "..\Shaders\Shader_Mipmap.h"


FrameBufferProcessor::FrameBufferProcessor()
{}


FrameBufferProcessor::~FrameBufferProcessor()
{}

void FrameBufferProcessor::GenerateBlurChain(FrameBuffer * buffer, RHICommandList * list)
{}

void FrameBufferProcessor::CreateMipChain(FrameBuffer * buffer, RHICommandList * list)
{
	const bool IsCompute = list->IsComputeList();
	if (!IsCompute)
	{
		buffer->MakeReadyForComputeUse(list);
	}
	Shader* s = ShaderComplier::GetShader<Shader_Mipmap>();
	RHIPipeLineStateDesc d = RHIPipeLineStateDesc::CreateDefault(s);
	list->SetPipelineStateDesc(d);
	RHIUAV* UAV = RHI::CreateUAV(nullptr);
	UAV->CreateUAVFromFrameBuffer(buffer, 0);
	const RHIFrameBufferDesc& Desc = buffer->GetDescription();
	for (int i = 0; i < Desc.MipCount - 1; i++)
	{
		int DstWidth = glm::max(Desc.Width >> (i + 1), 1);
		int DstHeight = glm::max(Desc.Height >> (i + 1), 1);
		UAV->CreateUAVFromFrameBuffer(buffer, i);
		UAV->Bind(list,s->GetSlotForName("DstTexture"));
		float INVWidth = 1.0f / DstWidth;
		list->SetRootConstant(s->GetSlotForName("CB"), 1, &INVWidth, 0);
		float INVHeight = 1.0f / DstHeight;
		list->SetRootConstant(s->GetSlotForName("CB"), 1, &INVHeight, 1);
		list->Dispatch(glm::max(DstWidth / 8, 1), glm::max(DstHeight / 8, 1), 1);
		list->UAVBarrier(UAV);
	}
}

void FrameBufferProcessor::BlurMipChain(FrameBuffer * buffer, RHICommandList * list)
{}
