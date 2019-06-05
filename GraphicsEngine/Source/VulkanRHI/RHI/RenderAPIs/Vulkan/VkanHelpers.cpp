#include "VulkanRHIPCH.h"
#include "VkanHelpers.h"
#include "VkanDeviceContext.h"
#include "vulkan/vulkan_core.h"

void VkanHelpers::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout StartingLayput)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(VKanRHI::RHIinstance->DevCon->device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(VKanRHI::RHIinstance->DevCon->device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(VKanRHI::RHIinstance->DevCon->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(VKanRHI::RHIinstance->DevCon->device, image, imageMemory, 0);
}

VkanHelpers::VkanHelpers()
{}


VkanHelpers::~VkanHelpers()
{}

void VkanHelpers::copyBuffer(VkCommandBuffer List, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(List, srcBuffer, dstBuffer, 1, &copyRegion);
}

void VkanHelpers::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VKanRHI::GetVDefaultDevice()->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(VKanRHI::GetVDefaultDevice()->device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(VKanRHI::GetVDefaultDevice()->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(VKanRHI::GetVDefaultDevice()->device, buffer, bufferMemory, 0);
}

void VkanHelpers::createImage(VkanDeviceContext* D, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout StartingLayput)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(D->device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(D->device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(D->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(D->device, image, imageMemory, 0);
}

uint32_t VkanHelpers::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(VKanRHI::GetVDefaultDevice()->physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void VkanHelpers::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

}

void VkanHelpers::copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{


	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


}

VkImageView VkanHelpers::createImageView(VkanDeviceContext* C, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(C->device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

VkFormat VkanHelpers::ConvertFormat(eTEXTURE_FORMAT format)
{
	switch (format)
	{
		case FORMAT_UNKNOWN:
			return VK_FORMAT_UNDEFINED;
			break;
		case FORMAT_R32G32B32A32_TYPELESS:
			break;
		case FORMAT_R32G32B32A32_FLOAT:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		case FORMAT_R32G32B32A32_UINT:
			return VK_FORMAT_R32G32B32_UINT;
		case FORMAT_R32G32B32A32_SINT:
			return VK_FORMAT_R32G32B32_SINT;
		case FORMAT_R32G32B32_TYPELESS:
			break;
		case FORMAT_R32G32B32_FLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case FORMAT_R32G32B32_UINT:
			break;
		case FORMAT_R32G32B32_SINT:
			break;
		case FORMAT_R16G16B16A16_TYPELESS:
			break;
		case FORMAT_R16G16B16A16_FLOAT:
			break;
		case FORMAT_R16G16B16A16_UNORM:
			break;
		case FORMAT_R16G16B16A16_UINT:
			break;
		case FORMAT_R16G16B16A16_SNORM:
			break;
		case FORMAT_R16G16B16A16_SINT:
			break;
		case FORMAT_R32G32_TYPELESS:
			break;
		case FORMAT_R32G32_FLOAT:
			return VK_FORMAT_R32G32_SFLOAT;

		case FORMAT_R32G32_UINT:
			return VK_FORMAT_R32G32_UINT;
		case FORMAT_R32G32_SINT:
			return VK_FORMAT_R32G32_SINT;
		case FORMAT_R32G8X24_TYPELESS:
			break;
		case FORMAT_D32_FLOAT_S8X24_UINT:
			break;
		case FORMAT_R32_FLOAT_X8X24_TYPELESS:
			break;
		case FORMAT_X32_TYPELESS_G8X24_UINT:
			break;
		case FORMAT_R10G10B10A2_TYPELESS:
			break;
		case FORMAT_R10G10B10A2_UNORM:
			break;
		case FORMAT_R10G10B10A2_UINT:
			break;
		case FORMAT_R11G11B10_FLOAT:
			break;
		case FORMAT_R8G8B8A8_TYPELESS:
			break;
		case FORMAT_R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case FORMAT_R8G8B8A8_UNORM_SRGB:
			break;
		case FORMAT_R8G8B8A8_UINT:
			break;
		case FORMAT_R8G8B8A8_SNORM:
			break;
		case FORMAT_R8G8B8A8_SINT:
			break;
		case FORMAT_R16G16_TYPELESS:
			break;
		case FORMAT_R16G16_FLOAT:
			break;
		case FORMAT_R16G16_UNORM:
			break;
		case FORMAT_R16G16_UINT:
			break;
		case FORMAT_R16G16_SNORM:
			break;
		case FORMAT_R16G16_SINT:
			break;
		case FORMAT_R32_TYPELESS:
			break;
		case FORMAT_D32_FLOAT:
			return VK_FORMAT_D32_SFLOAT;
			break;
		case FORMAT_R32_FLOAT:
			return VK_FORMAT_R32_SFLOAT;
			break;
		case FORMAT_R32_UINT:
			return VK_FORMAT_R32_UINT;
			break;
		case FORMAT_R32_SINT:
			return VK_FORMAT_R32_SINT;
			break;
		case FORMAT_R24G8_TYPELESS:
			break;
		case FORMAT_D24_UNORM_S8_UINT:
			break;
		case FORMAT_R24_UNORM_X8_TYPELESS:
			break;
		case FORMAT_X24_TYPELESS_G8_UINT:
			break;
		case FORMAT_R8G8_TYPELESS:
			break;
		case FORMAT_R8G8_UNORM:
			break;
		case FORMAT_R8G8_UINT:
			break;
		case FORMAT_R8G8_SNORM:
			break;
		case FORMAT_R8G8_SINT:
			break;
		case FORMAT_R16_TYPELESS:
			break;
		case FORMAT_R16_FLOAT:
			break;
		case FORMAT_D16_UNORM:
			break;
		case FORMAT_R16_UNORM:
			return VK_FORMAT_R16_UNORM;
			break;
		case FORMAT_R16_UINT:
			return VK_FORMAT_R16_UINT;
			break;
		case FORMAT_R16_SNORM:
			break;
		case FORMAT_R16_SINT:
			break;
		case FORMAT_R8_TYPELESS:
			break;
		case FORMAT_R8_UNORM:
			break;
		case FORMAT_R8_UINT:
			return VK_FORMAT_R8_UINT;
			break;
		case FORMAT_R8_SNORM:
			break;
		case FORMAT_R8_SINT:
			break;
		case FORMAT_A8_UNORM:
			break;
		case FORMAT_R1_UNORM:
			break;
		case FORMAT_R9G9B9E5_SHAREDEXP:
			break;
		case FORMAT_R8G8_B8G8_UNORM:
			break;
		case FORMAT_G8R8_G8B8_UNORM:
			break;
		case FORMAT_BC1_TYPELESS:
			break;
		case FORMAT_BC1_UNORM:
			break;
		case FORMAT_BC1_UNORM_SRGB:
			break;
		case FORMAT_BC2_TYPELESS:
			break;
		case FORMAT_BC2_UNORM:
			break;
		case FORMAT_BC2_UNORM_SRGB:
			break;
		case FORMAT_BC3_TYPELESS:
			break;
		case FORMAT_BC3_UNORM:
			break;
		case FORMAT_BC3_UNORM_SRGB:
			break;
		case FORMAT_BC4_TYPELESS:
			break;
		case FORMAT_BC4_UNORM:
			break;
		case FORMAT_BC4_SNORM:
			break;
		case FORMAT_BC5_TYPELESS:
			break;
		case FORMAT_BC5_UNORM:
			break;
		case FORMAT_BC5_SNORM:
			break;
		case FORMAT_B5G6R5_UNORM:
			break;
		case FORMAT_B5G5R5A1_UNORM:
			break;
		case FORMAT_B8G8R8A8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
			break;
		case FORMAT_B8G8R8X8_UNORM:
			break;
		case FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			break;
		case FORMAT_B8G8R8A8_TYPELESS:
			break;
		case FORMAT_B8G8R8A8_UNORM_SRGB:
			break;
		case FORMAT_B8G8R8X8_TYPELESS:
			break;
		case FORMAT_B8G8R8X8_UNORM_SRGB:
			break;
		case FORMAT_BC6H_TYPELESS:
			break;
		case FORMAT_BC6H_UF16:
			break;
		case FORMAT_BC6H_SF16:
			break;
		case FORMAT_BC7_TYPELESS:
			break;
		case FORMAT_BC7_UNORM:
			break;
		case FORMAT_BC7_UNORM_SRGB:
			break;
		case FORMAT_AYUV:
			break;
		case FORMAT_Y410:
			break;
		case FORMAT_Y416:
			break;
		case FORMAT_NV12:
			break;
		case FORMAT_P010:
			break;
		case FORMAT_P016:
			break;
		case FORMAT_420_OPAQUE:
			break;
		case FORMAT_YUY2:
			break;
		case FORMAT_Y210:
			break;
		case FORMAT_Y216:
			break;
		case FORMAT_NV11:
			break;
		case FORMAT_AI44:
			break;
		case FORMAT_IA44:
			break;
		case FORMAT_P8:
			break;
		case FORMAT_A8P8:
			break;
		case FORMAT_B4G4R4A4_UNORM:
			break;
		case FORMAT_FORCE_UINT:
			break;
		default:
			break;

	}
	ENUMCONVERTFAIL();
	return VkFormat();
}

VkImageLayout VkanHelpers::ConvertState(GPU_RESOURCE_STATES::Type state)
{
	switch (state)
	{
		case GPU_RESOURCE_STATES::RESOURCE_STATE_COMMON:
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_UNDEFINED:
			return VK_IMAGE_LAYOUT_UNDEFINED;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_INDEX_BUFFER:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_RENDER_TARGET:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_UNORDERED_ACCESS:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_DEPTH_WRITE:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_DEPTH_READ:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_STREAM_OUT:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_INDIRECT_ARGUMENT:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_COPY_DEST:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_COPY_SOURCE:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_RESOLVE_DEST:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_RESOLVE_SOURCE:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_GENERIC_READ:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_VIDEO_DECODE_READ:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_VIDEO_DECODE_WRITE:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_VIDEO_PROCESS_READ:
			break;
		case GPU_RESOURCE_STATES::RESOURCE_STATE_VIDEO_PROCESS_WRITE:
			break;
		default:
			break;

	}
	ENUMCONVERTFAIL();
	return VkImageLayout();
}
