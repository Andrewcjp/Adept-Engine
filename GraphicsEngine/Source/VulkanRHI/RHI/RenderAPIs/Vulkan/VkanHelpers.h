#pragma once
#include "VkanRHI.h"
class VkanHelpers
{
public:
	static void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkanHelpers();
	~VkanHelpers();
	static void copyBuffer(VkCommandBuffer List, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	static void createImage(VkanDeviceContext* D, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	static void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	static void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	static VkImageView createImageView(VkanDeviceContext * C, VkImage image, VkFormat format);
	static VkFormat ConvertFormat(eTEXTURE_FORMAT format);
};

