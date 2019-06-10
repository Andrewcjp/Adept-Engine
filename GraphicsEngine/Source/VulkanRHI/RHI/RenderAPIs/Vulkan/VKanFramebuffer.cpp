#include "VKanFramebuffer.h"
#include "Rendering/Core/FrameBuffer.h"
#include "RHI/RHIRenderPassCache.h"
#include "VkanDeviceContext.h"
#include "VkanHelpers.h"
#include "VkanPipeLineStateObject.h"
#include "VKanRHI.h"
#include "VKanCommandlist.h"
#include "VknGPUResource.h"

VKanFramebuffer::VKanFramebuffer(DeviceContext * device, const RHIFrameBufferDesc & Desc) :FrameBuffer(device, Desc)
{
	desc.NumRenderTargets = BufferDesc.RenderTargetCount;
	for (int i = 0; i < 8; i++)
	{
		desc.RTVFormats[i] = BufferDesc.RTFormats[i];
	}
	desc.DSVFormat = BufferDesc.DepthFormat;
	if (!BufferDesc.NeedsDepthStencil)
	{
		desc.DSVFormat = eTEXTURE_FORMAT::FORMAT_UNKNOWN;
	}

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
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		RTImages[i]->SetState(List, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
	WasTexture = false;
}

void VKanFramebuffer::TransitionTOPixel(VKanCommandlist* list)
{
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		RTImages[i]->SetState(list, VkanHelpers::ConvertState(GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
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
		for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
		{
			CreateRT(list, i);
			attachments.push_back(RTImageView[i]);
		}
	}

	if (BufferDesc.NeedsDepthStencil)
	{
		VkDeviceMemory Mem;
		VkImage DepthImage;
		VkFormat depthFormat = VkanHelpers::ConvertFormat(BufferDesc.DepthFormat);
		VkanHelpers::createImage(BufferDesc.Width, BufferDesc.Height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			DepthImage, Mem, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		depthImageView = VkanHelpers::createImageView(VKanRHI::VKConv(Device), DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		DepthResource = new VknGPUResource();
		DepthResource->Init(DepthImage, Mem, VK_IMAGE_LAYOUT_UNDEFINED, depthFormat);
		DepthResource->SetState(list, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		attachments.push_back(depthImageView);
	}

	//multiple attachments

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

void VKanFramebuffer::CreateRT(VKanCommandlist* list, int index)
{
	VkImage RTImage;
	VkDeviceMemory RTImageMemory;
	VkFormat fmt = VkanHelpers::ConvertFormat(BufferDesc.RTFormats[index]);
	VkanHelpers::createImage(BufferDesc.Width, BufferDesc.Height, fmt, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		RTImage, RTImageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	RTImages[index] = new VknGPUResource();
	RTImages[index]->Init(RTImage, RTImageMemory, VK_IMAGE_LAYOUT_UNDEFINED, fmt);
	RTImages[index]->SetState(list, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	RTImageView[index] = VkanHelpers::createImageView(VKanRHI::VKConv(Device), RTImages[index]->GetImage(), RTImages[index]->GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT);

}

void VKanFramebuffer::UpdateStateTrackingFromRP(RHIRenderPassDesc & Desc)
{
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		RTImages[i]->UpdateState(VkanHelpers::ConvertState(Desc.FinalState));
	}
}

Descriptor VKanFramebuffer::GetDescriptor(int slot, int resourceindex)
{
	Descriptor D = Descriptor(EDescriptorType::SRV);
	if (IsCreated)
	{
		D.ImageView = RTImageView[resourceindex];
	}
	else
	{
		D.ImageView = VK_NULL_HANDLE;
		LogEnsureMsgf(false, "Cannot bind a framebuffer that has not been created (used as RT once)");
	}
	D.bindpoint = slot;
	return D;
}