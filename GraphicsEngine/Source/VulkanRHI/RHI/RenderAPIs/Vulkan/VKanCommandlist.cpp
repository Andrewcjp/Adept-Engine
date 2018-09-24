#include "stdafx.h"
#include "VKanCommandlist.h"
#include "VkanBuffers.h"

VKanCommandlist::VKanCommandlist(ECommandListType::Type type, DeviceContext * context) :RHICommandList(type, context)
{


	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VKanRHI::RHIinstance->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(VKanRHI::RHIinstance->device, &allocInfo, &CommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

VKanCommandlist::~VKanCommandlist()
{}

void VKanCommandlist::ResetList()
{
	VKanRHI::RHIinstance->ReadyCmdList(&CommandBuffer);
}



void VKanCommandlist::SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ)
{}



void VKanCommandlist::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	vkCmdDraw(CommandBuffer, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void VKanCommandlist::DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation)
{}

void VKanCommandlist::SetVertexBuffer(RHIBuffer * buffer)
{
	VKanBuffer* vb = (VKanBuffer*)buffer;
	VkBuffer vertexBuffers[] = { vb->vertexbuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(CommandBuffer, 0, 1, vertexBuffers, offsets);

}

void VKanCommandlist::CreatePipelineState(Shader * shader, class FrameBuffer* Buffer)
{}

void VKanCommandlist::SetPipelineState(PipeLineState state)
{}

void VKanCommandlist::UpdateConstantBuffer(void * data, int offset)
{

}

void VKanCommandlist::SetConstantBufferView(RHIBuffer * buffer, int offset, int Register)
{

}

void VKanCommandlist::SetTexture(BaseTexture * texture, int slot)
{}


void VKanCommandlist::SetScreenBackBufferAsRT()
{}

void VKanCommandlist::ClearScreen()
{}

void VKanCommandlist::ClearFrameBuffer(FrameBuffer * buffer)
{}

void VKanCommandlist::UAVBarrier(RHIUAV * target)
{}

void VKanCommandlist::SetIndexBuffer(RHIBuffer * buffer)
{}

void VKanCommandlist::Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ)
{}

void VKanCommandlist::SetRenderTarget(FrameBuffer * target, int SubResourceIndex)
{}

void VKanCommandlist::Execute(DeviceContextQueue::Type Target /*= DeviceContextQueue::LIMIT*/)
{
	vkCmdEndRenderPass(CommandBuffer);
	if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
	VKanRHI::RHIinstance->ListcmdBuffers.push_back(CommandBuffer);

}

void VKanCommandlist::WaitForCompletion()
{}

void VKanCommandlist::SetPipelineStateObject(Shader * shader, FrameBuffer * Buffer/* = nullptr*/)
{}

void VKanCommandlist::SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex/* = 0*/)
{}

void VKanCommandlist::SetUpCommandSigniture(int commandSize, bool Dispatch)
{}

void VKanCommandlist::ExecuteIndiect(int MaxCommandCount, RHIBuffer * ArgumentBuffer, int ArgOffset, RHIBuffer * CountBuffer, int CountBufferOffset)
{}

void VKanCommandlist::SetRootConstant(int SignitureSlot, int ValueNum, void * Data, int DataOffset)
{}


void VkanUAV::Bind(RHICommandList * list, int slot)
{}

void VkanUAV::CreateUAVFromFrameBuffer(FrameBuffer * target)
{}

void VkanUAV::CreateUAVFromTexture(BaseTexture * target)
{}

void VkanUAV::CreateUAVFromRHIBuffer(RHIBuffer * target)
{}

void VkanTextureArray::AddFrameBufferBind(FrameBuffer * Buffer, int slot)
{}

void VkanTextureArray::BindToShader(RHICommandList * list, int slot)
{}

void VkanTextureArray::SetIndexNull(int TargetIndex)
{}
