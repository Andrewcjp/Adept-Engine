#pragma once
#include "vulkan/vulkan_core.h"
//holds a heap of memory
//Just before a draw allocates a descriptor set for the command to execute
//uses the already bound descriptor handles.
class VKNCommandlist;
class VKNDeviceContext;
class VKNDescriptorPool
{
public:
	VKNDescriptorPool(VKNDeviceContext* Con);
	~VKNDescriptorPool();
	void Init();
	void ResetAllocations();
	void AllocateAndBind(VKNCommandlist* List);
private:
	VkDescriptorSet AllocateSet(VKNCommandlist* list);
	void createDescriptorPool();
	VkDescriptorSet createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, int count);
	VkDescriptorPool descriptorPool;
	VKNDeviceContext* Context = nullptr;
	VkDescriptorSet LastUsedSet;
	CachedAllocator<VkDescriptorBufferInfo> BufferInfoAlloc;
	CachedAllocator< VkDescriptorImageInfo> ImageInfoAlloc;

	std::vector<VkWriteDescriptorSet> WriteData;
	std::vector<VkCopyDescriptorSet> CopyData;
};

