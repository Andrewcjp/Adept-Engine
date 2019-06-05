
#include "VKanCommandlist.h"
#include "VkanBuffers.h"
#include "RHI\RHITypes.h"
#include "VKanRHI.h"
#include "VkanDeviceContext.h"
#include "VkanPipeLineStateObject.h"
#if BUILD_VULKAN
#include "Core/Platform/PlatformCore.h"
#include "DescriptorPool.h"
#include "VKanTexture.h"
#include "RHI\RHIRenderPassCache.h"
#include "VKanFramebuffer.h"
VKanCommandlist::VKanCommandlist(ECommandListType::Type type, DeviceContext * context) :RHICommandList(type, context)
{
	Device = context;
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
	CurrentDescriptors.resize(25);
	ResetList();
}

VKanCommandlist::~VKanCommandlist()
{}

void VKanCommandlist::ResetList()
{
	CommandBuffer = Pools[VKanRHI::RHIinstance->currentFrame].Buffer;
	vkResetCommandPool(VKanRHI::GetVDefaultDevice()->device, Pools[VKanRHI::RHIinstance->currentFrame].Pool, 0);
	vkResetCommandBuffer(Pools[VKanRHI::RHIinstance->currentFrame].Buffer, 0);
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	if (vkBeginCommandBuffer(CommandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}
	IsOpen = true;
}

void VKanCommandlist::SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ)
{

}

void VKanCommandlist::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	ensure(IsOpen);
	VKanRHI::VKConv(Device)->pool->AllocateAndBind(this);
	ensure(IsInRenderPass);
	vkCmdDraw(CommandBuffer, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void VKanCommandlist::DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation)
{
	ensure(IsOpen);
	((VkanDeviceContext*)Device)->pool->AllocateAndBind(this);
	ensure(IsInRenderPass);
	vkCmdDrawIndexed(CommandBuffer, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void VKanCommandlist::SetVertexBuffer(RHIBuffer * buffer)
{
	ensure(IsOpen);
	VKanBuffer* vb = (VKanBuffer*)buffer;
	VkBuffer vertexBuffers[] = { vb->vertexbuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(CommandBuffer, 0, 1, vertexBuffers, offsets);
}

void VKanCommandlist::SetIndexBuffer(RHIBuffer * buffer)
{
	ensure(IsOpen);
	VKanBuffer* vb = (VKanBuffer*)buffer;
	vkCmdBindIndexBuffer(CommandBuffer, vb->vertexbuffer, 0, VK_INDEX_TYPE_UINT16);
}

void VKanCommandlist::SetConstantBufferView(RHIBuffer * buffer, int offset, int Register)
{
	ensure(IsOpen);
	ShaderParameter* Parm = CurrentPso->GetRootSigSlot(Register);
	VKanBuffer* V = (VKanBuffer*)buffer;
	CurrentDescriptors[Register] = V->GetDescriptor(Parm->RegisterSlot, offset);
}

void VKanCommandlist::SetTexture(BaseTextureRef texture, int slot)
{
	ensure(IsOpen);
	ShaderParameter* Parm = CurrentPso->GetRootSigSlot(slot);
	VKanTexture* V = (VKanTexture*)texture.Get();
	CurrentDescriptors[slot] = V->GetDescriptor(Parm->RegisterSlot);
}

VkCommandBuffer* VKanCommandlist::GetCommandBuffer()
{
	return &Pools[VKanRHI::RHIinstance->currentFrame].Buffer;
}

void VKanCommandlist::ClearFrameBuffer(FrameBuffer * buffer)
{}

void VKanCommandlist::UAVBarrier(RHIUAV * target)
{}


void VKanCommandlist::Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ)
{}

void VKanCommandlist::Execute(DeviceContextQueue::Type Target /*= DeviceContextQueue::LIMIT*/)
{
	if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &CommandBuffer;

	submitInfo.signalSemaphoreCount = 0;


	if (vkQueueSubmit(VKanRHI::VKConv(Device)->graphicsQueue, 1, &submitInfo, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	IsOpen = false;
}

void VKanCommandlist::BeginRenderPass(RHIRenderPassDesc& RenderPassInfo)
{
	ensure(IsOpen);
	RHICommandList::BeginRenderPass(RenderPassInfo);
	CurrnetRenderPass = VKanRHI::RHIinstance->Pass;
	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

	if (RenderPassInfo.TargetSwapChain)
	{
		renderPassInfo.renderPass = VKanRHI::VKConv(RHIRenderPassCache::Get()->GetOrCreatePass(RenderPassInfo))->RenderPass;
		renderPassInfo.framebuffer = VKanRHI::RHIinstance->swapChainFramebuffers[VKanRHI::RHIinstance->currentFrame];
	}
	else
	{
		VKanFramebuffer* FB = VKanRHI::VKConv(RenderPassInfo.TargetBuffer);
		FB->TryInitBuffer(RenderPassInfo, this);
		renderPassInfo.framebuffer = FB->Buffer;
		CurrnetRenderPass = VKanRHI::VKConv(RHIRenderPassCache::Get()->GetOrCreatePass(RenderPassInfo));
		renderPassInfo.renderPass = VKanRHI::VKConv(RHIRenderPassCache::Get()->GetOrCreatePass(RenderPassInfo))->RenderPass;
		//ensure(VKanRHI::VKConv(CurrentPso->GetDesc().RenderPass)->RenderPass == renderPassInfo.renderPass);
		if (CurrentPso != nullptr)
		{
			if (VKanRHI::VKConv(CurrentPso->GetDesc().RenderPass)->RenderPass != renderPassInfo.renderPass)
			{
				RHIPipeLineStateDesc NewDesc = CurrentPso->GetDesc();
				NewDesc.RenderPass = RHIRenderPassCache::Get()->GetOrCreatePass(RenderPassInfo);
				SetPipelineStateDesc(NewDesc);
				CurrnetRenderPass = VKanRHI::VKConv(NewDesc.RenderPass);
			}
		}
	}
	renderPassInfo.renderArea.offset = { 0, 0 };
	if (RenderPassInfo.TargetSwapChain)
	{
		renderPassInfo.renderArea.extent = VKanRHI::RHIinstance->swapChainExtent;
	}
	else
	{
		renderPassInfo.renderArea.extent.width = RenderPassInfo.TargetBuffer->GetWidth();
		renderPassInfo.renderArea.extent.height = RenderPassInfo.TargetBuffer->GetHeight();
	}


	VkClearValue clearColor[2];
	clearColor[0] = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearColor[1].depthStencil = { 1.0f,0 };
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = &clearColor[0];

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
	if (CurrentPso != nullptr && CurrentPso->Parms.size() != VObject->Parms.size())
	{
		CurrentDescriptors.clear();
		CurrentDescriptors.resize(25);
	}
	CurrentPso = VObject;
	ensure(CommandBuffer != nullptr);
	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VObject->Pipeline);
}

void VKanCommandlist::SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex/* = 0*/)
{
	if (Resourceindex == -1)
	{
		SetTexture(VKanRHI::RHIinstance->T, slot);
		VKanFramebuffer* V = VKanRHI::VKConv(buffer);
		V->WasTexture = true;
		return;
	}
	ShaderParameter* Parm = CurrentPso->GetRootSigSlot(slot);
	VKanFramebuffer* V = VKanRHI::VKConv(buffer);
	CurrentDescriptors[slot] = V->GetDescriptor(Parm->RegisterSlot);
	V->WasTexture = true;
}


void VKanCommandlist::SetHighLevelAccelerationStructure(HighLevelAccelerationStructure* Struct)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void VKanCommandlist::TraceRays(const RHIRayDispatchDesc& desc)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void VKanCommandlist::SetStateObject(RHIStateObject* Object)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void VKanCommandlist::SetUpCommandSigniture(int commandSize, bool Dispatch)
{}

void VKanCommandlist::ExecuteIndiect(int MaxCommandCount, RHIBuffer * ArgumentBuffer, int ArgOffset, RHIBuffer * CountBuffer, int CountBufferOffset)
{}

void VKanCommandlist::SetRootConstant(int SignitureSlot, int ValueNum, void * Data, int DataOffset)
{}


void VKanCommandlist::SetPipelineStateDesc(RHIPipeLineStateDesc& Desc)
{
	if (IsInRenderPass && CurrnetRenderPass != nullptr)
	{
		Desc.RenderPassDesc = CurrnetRenderPass->Desc;
		Desc.RenderPass = CurrnetRenderPass;
	}
	SetPipelineStateObject(Device->GetPSOCache()->GetFromCache(Desc));
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

void VkanTextureArray::SetFrameBufferFormat(RHIFrameBufferDesc & desc)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

#endif


