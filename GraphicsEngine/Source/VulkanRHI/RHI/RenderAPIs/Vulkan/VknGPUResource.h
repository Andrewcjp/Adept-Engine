#pragma once
#include "vulkan/vulkan_core.h"

class VKNCommandlist;
class VknGPUResource
{
public:
	VknGPUResource();
	~VknGPUResource();
	void Init(VkImage Image, VkDeviceMemory Memory, VkImageLayout State, VkFormat FMT);
	void SetState(VKNCommandlist* List, VkImageLayout Layout);
	VkImageLayout GetState()const;
	void UpdateState(VkImageLayout State);
	VkImage GetImage();
	VkFormat GetFormat();
	int Layers = 1;
private:
	VkFormat Format;
	VkImageLayout CurrentState;
	VkImage Image;
	VkDeviceMemory ImageMemory;
};

