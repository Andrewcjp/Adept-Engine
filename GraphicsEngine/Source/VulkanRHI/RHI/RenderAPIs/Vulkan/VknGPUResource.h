#pragma once
#include "vulkan/vulkan_core.h"

class VKanCommandlist;
class VknGPUResource
{
public:
	VknGPUResource();
	~VknGPUResource();
	void Init(VkImage Image, VkDeviceMemory Memory, VkImageLayout State, VkFormat FMT);
	void SetState(VKanCommandlist* List, VkImageLayout Layout);
	VkImageLayout GetState()const;
	void UpdateState(VkImageLayout State);
	VkImage GetImage();
	VkFormat GetFormat();
private:
	VkFormat Format;
	VkImageLayout CurrentState;
	VkImage Image;
	VkDeviceMemory ImageMemory;
};

