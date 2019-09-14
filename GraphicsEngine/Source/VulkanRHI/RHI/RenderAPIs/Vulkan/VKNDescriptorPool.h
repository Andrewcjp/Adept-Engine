#pragma once
#include "vulkan/vulkan_core.h"
template<class T>
class RapidVector
{
public:
	void Allocate(int count)
	{
		if (Array != nullptr)
		{
			delete[] Array;
		}
		ArraySize = count;
		Array = new T[count];
	}
	void push_back(const T& item)
	{
		Array[Index] = item;
		Index++;
	}
	void clear()
	{
		Index = 0;
	}
	int GetAllocsize() const
	{
		return ArraySize;
	}
	int size() const
	{
		return Index;
	}
	T* data()
	{
		return Array;
	}
private:
	int Index = 0;
	T* Array = nullptr;
	int ArraySize = 0;
};
#define USERV 1
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
	void AllocateAndBind(VKNCommandlist* RESTRICT List);
private:
	VkDescriptorSet AllocateSet(VKNCommandlist* RESTRICT list);
	void createDescriptorPool();
	VkDescriptorSet createDescriptorSets(const VkDescriptorSetLayout* RESTRICT descriptorSetLayout, int count);
	VkDescriptorPool descriptorPool;
	VKNDeviceContext* Context = nullptr;
	VkDescriptorSet LastUsedSet;
	CachedAllocator<VkDescriptorBufferInfo> BufferInfoAlloc;
	CachedAllocator< VkDescriptorImageInfo> ImageInfoAlloc;
#if 0
	std::vector<VkWriteDescriptorSet> WriteData;
	std::vector<VkCopyDescriptorSet> CopyData;
#else
	RapidVector<VkWriteDescriptorSet> WriteData;
	RapidVector<VkCopyDescriptorSet> CopyData;
#endif
};

