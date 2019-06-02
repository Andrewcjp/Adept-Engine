
#include "VKanFramebuffer.h"
#include "VkanPipeLineStateObject.h"
#include "VKanRHI.h"
#include "Rendering/Core/FrameBuffer.h"
#include "VkanHelpers.h"
#if BUILD_VULKAN

VKanFramebuffer::VKanFramebuffer(DeviceContext * device, const RHIFrameBufferDesc & Desc) :FrameBuffer(device, Desc)
{

}
DeviceContext * VKanFramebuffer::GetDevice()
{
	return nullptr;
}

const RHIPipeRenderTargetDesc & VKanFramebuffer::GetPiplineRenderDesc()
{
	return desc;
}

void VKanFramebuffer::MakeReadyForComputeUse(RHICommandList* List, bool Depth /*= false*/)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void VKanFramebuffer::MakeReadyForCopy(RHICommandList * list)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void VKanFramebuffer::TryInitBuffer(VKanRenderPass * RenderPass)
{
	if (IsCreated)
	{
		return;
	}
	IsCreated = true;

#if 0
	VkImageView attachments[] = {
		swapChainImageViews[i]
	};

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = RenderPass->RenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.width = BufferDesc.Width;
	framebufferInfo.height = BufferDesc.Height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(VKanRHI::VKConv(Device)->Device, &framebufferInfo, nullptr, &Buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create framebuffer!");
	}
#endif


	if (BufferDesc.NeedsDepthStencil)
	{
		VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
		VkanHelpers::createImage(BufferDesc.Width, BufferDesc.Height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		depthImageView = VkanHelpers::createImageView(VKanRHI::VKConv(Device), depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

}
#endif