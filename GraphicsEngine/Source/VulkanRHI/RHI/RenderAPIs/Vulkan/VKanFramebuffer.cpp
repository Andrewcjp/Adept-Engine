#include "VKanFramebuffer.h"
#include "Rendering/Core/FrameBuffer.h"
#include "RHI/RHIRenderPassCache.h"
#include "VkanDeviceContext.h"
#include "VkanHelpers.h"
#include "VkanPipeLineStateObject.h"
#include "VKanRHI.h"
#include "VKanCommandlist.h"

VKanFramebuffer::VKanFramebuffer(DeviceContext * device, const RHIFrameBufferDesc & Desc) :FrameBuffer(device, Desc)
{
	desc.NumRenderTargets = BufferDesc.RenderTargetCount;
	desc.RTVFormats[0] = BufferDesc.RTFormats[0];
	desc.DSVFormat = BufferDesc.DepthFormat;
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

void VKanFramebuffer::UnBind(VKanCommandlist * List)
{
	VkFormat fmt = VkanHelpers::ConvertFormat(BufferDesc.RTFormats[0]);
	VkanHelpers::transitionImageLayout(*List->GetCommandBuffer(), RTImage, fmt, VkanHelpers::ConvertState(GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void VKanFramebuffer::MakeReadyForCopy(RHICommandList * list)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void VKanFramebuffer::TryInitBuffer(RHIRenderPassDesc& desc)
{
	if (IsCreated)
	{
		return;
	}
	IsCreated = true;
	Device = RHI::GetDefaultDevice();
	std::vector<VkImageView> attachments;
	desc.Build();
	if (BufferDesc.RenderTargetCount > 0)
	{
		VkFormat fmt = VkanHelpers::ConvertFormat(BufferDesc.RTFormats[0]);
		VkanHelpers::createImage(BufferDesc.Width, BufferDesc.Height, fmt, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, RTImage, RTImageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		RTImageView = VkanHelpers::createImageView(VKanRHI::VKConv(Device), RTImage, fmt, VK_IMAGE_ASPECT_COLOR_BIT);
		VkCommandBuffer B = VKanRHI::RHIinstance->setuplist->CommandBuffer;
		VkanHelpers::transitionImageLayout(B, RTImage, fmt, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		attachments.push_back(RTImageView);
	}

	if (BufferDesc.NeedsDepthStencil)
	{
		VkFormat depthFormat = VkanHelpers::ConvertFormat(BufferDesc.DepthFormat);
		VkanHelpers::createImage(BufferDesc.Width, BufferDesc.Height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		depthImageView = VkanHelpers::createImageView(VKanRHI::VKConv(Device), depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		VkCommandBuffer B = VKanRHI::RHIinstance->setuplist->CommandBuffer;
		VkanHelpers::transitionImageLayout(B, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		attachments.push_back(depthImageView);
	}



	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = VKanRHI::VKConv(RHIRenderPassCache::Get()->GetOrCreatePass(desc))->RenderPass;
	framebufferInfo.attachmentCount = attachments.size();
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = BufferDesc.Width;
	framebufferInfo.height = BufferDesc.Height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(VKanRHI::VKConv(Device)->device, &framebufferInfo, nullptr, &Buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create framebuffer!");
	}
}

Descriptor VKanFramebuffer::GetDescriptor(int slot)
{
	Descriptor D = Descriptor(EDescriptorType::SRV);
	D.ImageView = RTImageView;
	D.bindpoint = slot;
	return D;
}