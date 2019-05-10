
#include "VKanCommandlist.h"
#include "VkanBuffers.h"
#include "RHI\RHITypes.h"
#include "VKanRHI.h"
#include "VkanDeviceContext.h"
#include "VkanPipeLineStateObject.h"
#if BUILD_VULKAN
#include "Core/Platform/PlatformCore.h"
VKanCommandlist::VKanCommandlist(ECommandListType::Type type, DeviceContext * context) :RHICommandList(type, context)
{

	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		Pools[i].Pool = VKanRHI::RHIinstance->createCommandPool();

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = Pools[i].Pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(VKanRHI::GetVDefaultDevice()->device, &allocInfo, &Pools[i].Buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

}

VKanCommandlist::~VKanCommandlist()
{}

void VKanCommandlist::ResetList()
{
	CommandBuffer = Pools[VKanRHI::RHIinstance->currentFrame].Buffer;
	vkResetCommandPool(VKanRHI::GetVDefaultDevice()->device, Pools[VKanRHI::RHIinstance->currentFrame].Pool, 0);
	vkResetCommandBuffer(Pools[VKanRHI::RHIinstance->currentFrame].Buffer, 0);
	VKanRHI::RHIinstance->ReadyCmdList(&Pools[VKanRHI::RHIinstance->currentFrame].Buffer);

}

void VKanCommandlist::SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ)
{}

void VKanCommandlist::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	ensure(IsInRenderPass);
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


void VKanCommandlist::SetConstantBufferView(RHIBuffer * buffer, int offset, int Register)
{

}

void VKanCommandlist::SetTexture(BaseTexture * texture, int slot)
{}

VkCommandBuffer* VKanCommandlist::GetCommandBuffer()
{
	return &Pools[VKanRHI::RHIinstance->currentFrame].Buffer;
}




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

	if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
	//VKanRHI::RHIinstance->ListcmdBuffers.push_back(CommandBuffer);

}

void VKanCommandlist::BeginRenderPass(RHIRenderPassInfo& RenderPassInfo)
{
	RHICommandList::BeginRenderPass(RenderPassInfo);
	CurrnetRenderPass = (VKanRenderPass*)RenderPassInfo.Pass;
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = CurrnetRenderPass->RenderPass;

	renderPassInfo.framebuffer = VKanRHI::RHIinstance->swapChainFramebuffers[VKanRHI::RHIinstance->currentFrame];

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = VKanRHI::RHIinstance->swapChainExtent;

	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VKanCommandlist::EndRenderPass()
{
	RHICommandList::EndRenderPass();
	vkCmdEndRenderPass(CommandBuffer);
}

void VKanCommandlist::SetPipelineStateObject(RHIPipeLineStateObject* Object)
{
	VkanPipeLineStateObject* VObject = (VkanPipeLineStateObject*)Object;

	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VObject->Pipeline);
}

void VKanCommandlist::SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex/* = 0*/)
{}


void VKanCommandlist::SetUpCommandSigniture(int commandSize, bool Dispatch)
{}

void VKanCommandlist::ExecuteIndiect(int MaxCommandCount, RHIBuffer * ArgumentBuffer, int ArgOffset, RHIBuffer * CountBuffer, int CountBufferOffset)
{}

void VKanCommandlist::SetRootConstant(int SignitureSlot, int ValueNum, void * Data, int DataOffset)
{}


void VKanCommandlist::SetPipelineStateDesc(RHIPipeLineStateDesc& Desc)
{

}

void VkanUAV::Bind(RHICommandList * list, int slot)
{}

void VkanUAV::CreateUAVFromFrameBuffer(FrameBuffer * target, int mip)
{}

void VkanUAV::CreateUAVFromTexture(BaseTexture * target)
{}

void VkanUAV::CreateUAVFromRHIBuffer(RHIBuffer * target)
{}

void VkanTextureArray::AddFrameBufferBind(FrameBuffer * Buffer, int slot)
{}

void VkanTextureArray::BindToShader(RHICommandList * list, int slot)
{}

void VkanTextureArray::SetIndexNull(int TargetIndex, FrameBuffer* Buffer /*= nullptr*/)
{

}

void VkanTextureArray::Clear()
{

}
#endif


