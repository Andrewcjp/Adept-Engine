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
	//throw std::logic_error("The method or operation is not implemented.");
}

void VKanFramebuffer::UnBind(VKanCommandlist * List)
{
	if (!WasTexture)
	{
		return;
	}
	VkFormat fmt = VkanHelpers::ConvertFormat(BufferDesc.RTFormats[0]);
	VkanHelpers::transitionImageLayout(*List->GetCommandBuffer(), RTImage, fmt, VkanHelpers::ConvertState(GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	WasTexture = false;
}
void VKanFramebuffer::TransitionTOPixel(VKanCommandlist* list)
{
	VkFormat fmt = VkanHelpers::ConvertFormat(BufferDesc.RTFormats[0]);
	VkanHelpers::transitionImageLayout(*list->GetCommandBuffer(), RTImage, fmt, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VkanHelpers::ConvertState(GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}
void VKanFramebuffer::MakeReadyForCopy(RHICommandList * list)
{
	//throw std::logic_error("The method or operation is not implemented.");
}

void VKanFramebuffer::TryInitBuffer(RHIRenderPassDesc& RPdesc, VKanCommandlist* list)
{
	if (IsCreated)
	{
		UnBind(list);
		return;
	}
	IsCreated = true;
	Device = RHI::GetDefaultDevice();
	std::vector<VkImageView> attachments;
	RPdesc.Build();
	if (BufferDesc.RenderTargetCount > 0)
	{
		VkFormat fmt = VkanHelpers::ConvertFormat(BufferDesc.RTFormats[0]);
		VkanHelpers::createImage(BufferDesc.Width, BufferDesc.Height, fmt, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, RTImage, RTImageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		RTImageView = VkanHelpers::createImageView(VKanRHI::VKConv(Device), RTImage, fmt, VK_IMAGE_ASPECT_COLOR_BIT);
		VkCommandBuffer B = *list->GetCommandBuffer();// VKanRHI::RHIinstance->setuplist->CommandBuffer;
		VkanHelpers::transitionImageLayout(B, RTImage, fmt, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		attachments.push_back(RTImageView);
	}

	if (BufferDesc.NeedsDepthStencil)
	{
		VkFormat depthFormat = VkanHelpers::ConvertFormat(BufferDesc.DepthFormat);
		VkanHelpers::createImage(BufferDesc.Width, BufferDesc.Height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		depthImageView = VkanHelpers::createImageView(VKanRHI::VKConv(Device), depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		VkCommandBuffer B = *list->GetCommandBuffer();// VKanRHI::RHIinstance->setuplist->CommandBuffer;
		VkanHelpers::transitionImageLayout(B, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		attachments.push_back(depthImageView);
	}



	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = VKanRHI::VKConv(RHIRenderPassCache::Get()->GetOrCreatePass(RPdesc))->RenderPass;
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
	if (IsCreated)
	{
		D.ImageView = RTImageView;
	}
	else
	{
		D.ImageView = VK_NULL_HANDLE;
		LogEnsureMsgf(false, "Cannot bind a framebuffer that has not been created (used as RT once)");
	}
	D.bindpoint = slot;
	return D;
}