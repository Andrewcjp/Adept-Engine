#pragma once
#include "vulkan/vulkan_core.h"
//holds a heap of memory
//Just before a draw allocates a descriptor set for the command to execute
//uses the already bound descriptor handles.
class VKanCommandlist;
class VkanDeviceContext;
class DescriptorPool
{
public:
	DescriptorPool(VkanDeviceContext* Con);
	~DescriptorPool();
	void Init();
	void ResetAllocations();
	void AllocateAndBind(VKanCommandlist* List);
private:
	VkDescriptorSet AllocateSet(VKanCommandlist* list);
	void createDescriptorPool();
	VkDescriptorSet createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, int count);
	VkDescriptorPool descriptorPool;
	VkanDeviceContext* Context = nullptr;
};

