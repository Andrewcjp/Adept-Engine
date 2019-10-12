#include "VKNRenderPass.h"
#include "VKNHelpers.h"
#include "VKNDeviceContext.h"

VKNRenderPass::VKNRenderPass(const RHIRenderPassDesc & desc, DeviceContext * Device) :RHIRenderPass(desc)
{
	VDevice = VKNRHI::VKConv(Device);
}
VkAttachmentLoadOp ConvertLoadOp(ERenderPassLoadOp::Type Op)
{
	switch (Op)
	{
		case ERenderPassLoadOp::Clear:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case ERenderPassLoadOp::DontCare:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		case ERenderPassLoadOp::Load:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
	}
	ENUMCONVERTFAIL();
	return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
}

VkAttachmentStoreOp ConvertStoreOp(ERenderPassStoreOp::Type Op)
{
	switch (Op)
	{
		case ERenderPassStoreOp::Store:
			return VK_ATTACHMENT_STORE_OP_STORE;
		case ERenderPassStoreOp::DontCare:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
	ENUMCONVERTFAIL();
	return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
}


void VKNRenderPass::Complie()
{
	Desc.Build();
	std::vector<VkAttachmentDescription> ColorAttamentsDesc;
	for (int i = 0; i < Desc.RenderDesc.NumRenderTargets; i++)
	{
		if (Desc.RenderDesc.RTVFormats[i] == eTEXTURE_FORMAT::FORMAT_UNKNOWN)
		{
			continue;
		}
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = VKNHelpers::ConvertFormat(Desc.RenderDesc.RTVFormats[i]);
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = ConvertLoadOp(Desc.LoadOp);
		colorAttachment.storeOp = ConvertStoreOp(Desc.StoreOp);
		colorAttachment.stencilLoadOp = ConvertLoadOp(Desc.StencilLoadOp);
		colorAttachment.stencilStoreOp = ConvertStoreOp(Desc.StencilStoreOp);
		colorAttachment.initialLayout = VKNHelpers::ConvertState(Desc.InitalState);
		colorAttachment.finalLayout = VKNHelpers::ConvertState(Desc.FinalState);
		ColorAttamentsDesc.push_back(colorAttachment);
	}
	if (Desc.RenderDesc.DSVFormat != eTEXTURE_FORMAT::FORMAT_UNKNOWN)
	{
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = ConvertLoadOp(Desc.LoadOp);
		depthAttachment.storeOp = ConvertStoreOp(Desc.StoreOp);
		depthAttachment.stencilLoadOp = ConvertLoadOp(Desc.StencilLoadOp);
		depthAttachment.stencilStoreOp = ConvertStoreOp(Desc.StencilStoreOp);
		depthAttachment.format = VKNHelpers::ConvertFormat(Desc.RenderDesc.DSVFormat);
		if (Desc.RenderDesc.NumRenderTargets == 0)
		{
			depthAttachment.initialLayout = VKNHelpers::MakeSafeForDepth(VKNHelpers::ConvertState(Desc.InitalState));
			depthAttachment.finalLayout = VKNHelpers::MakeSafeForDepth(VKNHelpers::ConvertState(Desc.FinalState));
		}
		else
		{
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		ColorAttamentsDesc.push_back(depthAttachment);
	}

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	std::vector< VkAttachmentReference> Attchments;
	for (int i = 0; i < Desc.RenderDesc.NumRenderTargets; i++)
	{
		if (Desc.RenderDesc.RTVFormats[i] == eTEXTURE_FORMAT::FORMAT_UNKNOWN)
		{
			continue;
		}
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = Attchments.size();
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		Attchments.push_back(colorAttachmentRef);
	}
	subpass.colorAttachmentCount = Desc.RenderDesc.NumRenderTargets;
	subpass.pColorAttachments = Attchments.data();

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = Desc.RenderDesc.NumRenderTargets;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	if (Desc.RenderDesc.DSVFormat != eTEXTURE_FORMAT::FORMAT_UNKNOWN)
	{
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
	}

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = ColorAttamentsDesc.size();
	renderPassInfo.pAttachments = ColorAttamentsDesc.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(VDevice->device, &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}

}


