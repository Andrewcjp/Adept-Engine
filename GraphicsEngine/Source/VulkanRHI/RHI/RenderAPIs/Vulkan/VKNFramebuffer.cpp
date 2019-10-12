#include "VKNFramebuffer.h"
#include "Rendering/Core/FrameBuffer.h"
#include "RHI/RHIRenderPassCache.h"
#include "VKNDeviceContext.h"
#include "VKNHelpers.h"
#include "VKNPipeLineStateObject.h"
#include "VKNRHI.h"
#include "VKNCommandlist.h"
#include "VknGPUResource.h"
#include "VKNRenderPass.h"

VKNFramebuffer::VKNFramebuffer(DeviceContext * device, const RHIFrameBufferDesc & Desc) :FrameBuffer(device, Desc)
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
DeviceContext * VKNFramebuffer::GetDevice()
{
	return nullptr;
}

const RHIPipeRenderTargetDesc & VKNFramebuffer::GetPiplineRenderDesc()
{
	return desc;
}

void VKNFramebuffer::UnBind(VKNCommandlist * List)
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

void VKNFramebuffer::TransitionTOPixel(VKNCommandlist* list)
{
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		RTImages[i]->SetState(list, VKNHelpers::ConvertState(GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
}

VkImageLayout ConvertResourceState(EResourceState::Type state)
{
	switch (state)
	{
		case EResourceState::RenderTarget:
			return VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case EResourceState::PixelShader:
			return VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case EResourceState::ComputeUse:
		case EResourceState::UAV:
			return VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;//i see		
		case EResourceState::CopySrc:
			return VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case EResourceState::CopyDst:
			return VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	}

	return VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
}
void VKNFramebuffer::SetResourceState(RHICommandList* List, EResourceState::Type State, bool ChangeDepth /*= false*/)
{
	if (State != EResourceState::PixelShader && State != EResourceState::RenderTarget)
	{
		return;
	}
	VKNCommandlist* VList = VKNRHI::VKConv(List);
	for (int i = 0; i < desc.NumRenderTargets; i++)
	{
		RTImages[i]->SetState(VList, ConvertResourceState(State));
	}
	if (ChangeDepth)
	{
		VkImageLayout layout = ConvertResourceState(State);
		if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		DepthResource->SetState(VList, layout);
	}
}

void VKNFramebuffer::TryInitBuffer(const RHIRenderPassDesc& RPdesc, VKNCommandlist* list)
{
	if (IsCreated)
	{
		UnBind(list);
		return;
	}
	IsCreated = true;
	Device = RHI::GetDefaultDevice();
	std::vector<VkImageView> attachments;
	RHIRenderPassDesc De = RPdesc; 
	De.Build();
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
		VkFormat depthFormat = VKNHelpers::ConvertFormat(BufferDesc.DepthFormat);
		VKNHelpers::createImage(BufferDesc.Width, BufferDesc.Height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			DepthImage, Mem, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, BufferDesc.TextureDepth);
		depthImageView = VKNHelpers::createImageView(VKNRHI::VKConv(Device), DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, BufferDesc.TextureDepth);
		DepthResource = new VknGPUResource();
		DepthResource->Init(DepthImage, Mem, VK_IMAGE_LAYOUT_UNDEFINED, depthFormat);
		DepthResource->Layers = BufferDesc.TextureDepth;
		DepthResource->SetState(list, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		attachments.push_back(depthImageView);
	}

	//multiple attachments

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = VKNRHI::VKConv(RHIRenderPassCache::Get()->GetOrCreatePass(De))->RenderPass;
	framebufferInfo.attachmentCount = attachments.size();
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = BufferDesc.Width;
	framebufferInfo.height = BufferDesc.Height;
	framebufferInfo.layers = BufferDesc.TextureDepth;

	if (vkCreateFramebuffer(VKNRHI::VKConv(Device)->device, &framebufferInfo, nullptr, &Buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create framebuffer!");
	}
}

void VKNFramebuffer::CreateRT(VKNCommandlist* list, int index)
{
	VkImage RTImage;
	VkDeviceMemory RTImageMemory;
	VkFormat fmt = VKNHelpers::ConvertFormat(BufferDesc.RTFormats[index]);
	VKNHelpers::createImage(BufferDesc.Width, BufferDesc.Height, fmt, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		RTImage, RTImageMemory, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, BufferDesc.TextureDepth);

	RTImages[index] = new VknGPUResource();
	RTImages[index]->Layers = BufferDesc.TextureDepth;
	RTImages[index]->Init(RTImage, RTImageMemory, VK_IMAGE_LAYOUT_UNDEFINED, fmt);
	RTImages[index]->SetState(list, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	RTImageView[index] = VKNHelpers::createImageView(VKNRHI::VKConv(Device), RTImages[index]->GetImage(), RTImages[index]->GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT, BufferDesc.TextureDepth);
#if NAME_RHI_PRIMS
	std::string name = std::string("VK") + GetDebugName();
	VkDebugUtilsObjectNameInfoEXT info = {};
	info.objectHandle = (uint64_t)RTImage;
	info.pObjectName = name.c_str();
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	PFN_vkSetDebugUtilsObjectNameEXT CreateDebugReportCallback = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(VKNRHI::RHIinstance->instance, "vkSetDebugUtilsObjectNameEXT");
	if (CreateDebugReportCallback)
	{
		CreateDebugReportCallback(VKNRHI::RHIinstance->GetVDefaultDevice()->device, &info);
	}
#endif
}

void VKNFramebuffer::UpdateStateTrackingFromRP(RHIRenderPassDesc & Desc)
{
	for (int i = 0; i < BufferDesc.RenderTargetCount; i++)
	{
		RTImages[i]->UpdateState(VKNHelpers::ConvertState(Desc.FinalState));
	}
}

Descriptor VKNFramebuffer::GetDescriptor(int slot, int resourceindex)
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