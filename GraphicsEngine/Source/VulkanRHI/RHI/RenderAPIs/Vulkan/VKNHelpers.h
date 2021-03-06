#pragma once
#include "VKNRHI.h"

struct TextureDescription;
class VKNHelpers
{
public:
	static void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout StartingLayput = VK_IMAGE_LAYOUT_UNDEFINED, int depth = 1);
	static void createImageDesc(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory, VkImageLayout StartingLayput, TextureDescription & desc);
	VKNHelpers();
	~VKNHelpers();
	static void copyBuffer(VkCommandBuffer List, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static void createImage(VKNDeviceContext* D, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout StartingLayput = VK_IMAGE_LAYOUT_UNDEFINED);
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, int levels = 1, int layers = 1);
	static void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	static VkImageView createImageView(VKNDeviceContext * C, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, int layer = 1);
	static VkImageView createImageView(VKNDeviceContext * C, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, TextureDescription & desc);
	static VkFormat ConvertFormat(eTEXTURE_FORMAT format);
	static VkImageLayout ConvertState(GPU_RESOURCE_STATES::Type state);
	static UINT Align(UINT size, UINT alignment = 256);
	static VkImageLayout MakeSafeForDepth(VkImageLayout Layout);
};

