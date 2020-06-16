#include "FrameBufferProcessor.h"
#include "Core/Assets/ShaderCompiler.h"
#include "FrameBuffer.h"
#include "Rendering/Shaders/Shader_Mipmap.h"
#include "RHI/RHICommandList.h"
#include "../Shaders/Shader_BlurCubemap.h"

FrameBufferProcessor::FrameBufferProcessor()
{}


FrameBufferProcessor::~FrameBufferProcessor()
{}

void FrameBufferProcessor::GenerateBlurChain(FrameBuffer * buffer, RHICommandList * list)
{
	return;
	const bool IsCompute = list->IsComputeList();
	if (!IsCompute)
	{
		buffer->MakeReadyForComputeUse(list);
	}

	Shader* s = ShaderCompiler::GetShader<Shader_BlurCubemap>();
	RHIPipeLineStateDesc d = RHIPipeLineStateDesc::CreateDefault(s);
	list->SetPipelineStateDesc(d);
#if 0
	//list->SetFrameBufferTexture(buffer, "");
	RHIUAV* UAV = nullptr;
	const RHIFrameBufferDesc& Desc = buffer->GetDescription();
	list->SetFrameBufferTexture(buffer, "Cubemap");
	for (int x = 0; x < Desc.TextureDepth; x++)
	{
		for (int i = 0; i < Desc.MipCount - 1; i++)
		{
			RHIViewDesc VDesc;
			VDesc.Mip = i + 1;
			VDesc.Dimension = DIMENSION_TEXTURE2DARRAY;
			VDesc.ArraySlice = x;
			VDesc.ViewType = EViewType::UAV;
			UAV = buffer->GetUAV(VDesc);
			int DstWidth = glm::max(Desc.Width >> (i + 1), 1);
			int DstHeight = glm::max(Desc.Height >> (i + 1), 1);
			UAV->Bind(list, s->GetSlotForName("DstTexture"));
			Shader_BlurCubemap::SData D;
			D.Mip = 0;
			D.Threads = glm::ivec2(glm::max(DstWidth / 8, 1), glm::max(DstHeight / 8, 1));
			list->SetRootConstant(s->GetSlotForName("CB"), 1, &D, 0);
			list->Dispatch(glm::max(DstWidth / 8, 1), glm::max(DstHeight / 8, 1), 1);
			list->UAVBarrier(UAV);
		}
	}
#endif
}

void FrameBufferProcessor::CreateMipChain(FrameBuffer * buffer, RHICommandList * list)
{
	return;
	const bool IsCompute = list->IsComputeList();
	if (!IsCompute)
	{
		buffer->MakeReadyForComputeUse(list);
	}

	Shader* s = ShaderCompiler::GetShader<Shader_Mipmap>();
	RHIPipeLineStateDesc d = RHIPipeLineStateDesc::CreateDefault(s);
	list->SetPipelineStateDesc(d);
	//list->SetFrameBufferTexture(buffer, "");

	const RHIFrameBufferDesc& Desc = buffer->GetDescription();

	for (int x = 0; x < Desc.TextureDepth; x++)
	{
		for (int i = 0; i < Desc.MipCount - 1; i++)
		{
			RHIViewDesc VDesc;
			VDesc.Mip = i + 1;
			VDesc.Dimension = DIMENSION_TEXTURE2DARRAY;
			VDesc.ArraySlice = x;
			VDesc.ViewType = EViewType::UAV;
		
			int DstWidth = glm::max(Desc.Width >> (i + 1), 1);
			int DstHeight = glm::max(Desc.Height >> (i + 1), 1);

			VDesc.Mip = i;
			//list->BindSRV(buffer, 1, VDesc);
			//#todo view test
			/*	UAV->Bind(list, s->GetSlotForName("DstTexture"));
				list->SetUAV(InputTexture, 1);*/
			float INVWidth = 1.0f / DstWidth;
			list->SetRootConstant(s->GetSlotForName("CB"), 1, &INVWidth, 0);
			float INVHeight = 1.0f / DstHeight;
			list->SetRootConstant(s->GetSlotForName("CB"), 1, &INVHeight, 1);
			list->Dispatch(glm::max(DstWidth / 8, 1), glm::max(DstHeight / 8, 1), 1);
		//	list->UAVBarrier(UAV);
		}
	}
}

void FrameBufferProcessor::BlurMipChain(FrameBuffer * buffer, RHICommandList * list)
{}
