#include "VulkanRHIPCH.h"
#include "DescriptorPool.h"
#include "VKanRHI.h"
#include "VkanDeviceContext.h"
#include "VKanCommandlist.h"
#include "VkanPipeLineStateObject.h"
#include "VkanBuffers.h"
#include "Descriptor.h"
#include "VKanTexture.h"


DescriptorPool::DescriptorPool(VkanDeviceContext* Con)
{
	Context = Con;
	Init();
}


DescriptorPool::~DescriptorPool()
{}

void DescriptorPool::Init()
{
	createDescriptorPool();
}

void DescriptorPool::ResetAllocations()
{
	vkResetDescriptorPool(Context->device, descriptorPool, 0);
}

void DescriptorPool::AllocateAndBind(VKanCommandlist * List)
{
	VkDescriptorSet Set = AllocateSet(List);
	vkCmdBindDescriptorSets(List->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, List->CurrentPso->PipelineLayout, 0, 1, &Set, 0, nullptr);
}
#define STATIC_SAMPLER 1
VkDescriptorSet DescriptorPool::AllocateSet(VKanCommandlist * list)
{
	VkDescriptorSet Set = createDescriptorSets(list->CurrentPso->descriptorSetLayout, 1);
	std::vector<VkWriteDescriptorSet> WriteData;
	for (int i = 0; i < list->CurrentDescriptors.size(); i++)
	{
		Descriptor* Desc = &list->CurrentDescriptors[i];
		if (Desc->DescType == EDescriptorType::Limit)
		{
			continue;
		}
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = Set;
		descriptorWrite.dstBinding = Desc->bindpoint;
		if (Desc->bindpoint == 2)
		{
			descriptorWrite.dstBinding = 3;
		}
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorCount = 1;

		if (Desc->DescType == EDescriptorType::CBV)
		{
			VkDescriptorBufferInfo* bufferInfo = new VkDescriptorBufferInfo();
			bufferInfo->buffer = Desc->Buffer->vertexbuffer;
			bufferInfo->offset = Desc->Offset;
			bufferInfo->range = Desc->Buffer->GetSize();
			descriptorWrite.pBufferInfo = bufferInfo;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
		else if (Desc->DescType == EDescriptorType::SRV)
		{
			VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
			imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (Desc->Texture == nullptr)
			{
				imageInfo->imageView = Desc->ImageView;
			}
			else
			{
				imageInfo->imageView = Desc->Texture->textureImageView;
			}

			imageInfo->sampler = list->CurrentPso->textureSampler;
			descriptorWrite.pImageInfo = imageInfo;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		}
		WriteData.push_back(descriptorWrite);
	}
	vkUpdateDescriptorSets(Context->device, WriteData.size(), WriteData.data(), 0, nullptr);
	for (int i = 0; i < WriteData.size(); i++)
	{
		SafeDelete(WriteData[i].pImageInfo);
		SafeDelete(WriteData[i].pBufferInfo);
	}
	return Set;
}

void DescriptorPool::createDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> Sizes;
	int MaxCount = 1000;
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = MaxCount;
	Sizes.push_back(poolSize);
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	Sizes.push_back(poolSize);
	poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLER;
	Sizes.push_back(poolSize);
	poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	Sizes.push_back(poolSize);


	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = Sizes.size();
	poolInfo.pPoolSizes = Sizes.data();
	poolInfo.maxSets = 500;

	if (vkCreateDescriptorPool(Context->device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

VkDescriptorSet DescriptorPool::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, int count)
{

	///vkResetDescriptorPool(Context->device, descriptorPool, 0);
	std::vector<VkDescriptorSetLayout> layouts(count, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = count;
	allocInfo.pSetLayouts = layouts.data();
	std::vector<VkDescriptorSet> descriptorSets;
	descriptorSets.resize(count);
	if (vkAllocateDescriptorSets(Context->device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}




#if 0

	for (size_t i = 0; i < 2; i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = VKanRHI::RHIinstance->buffer->vertexbuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = VKanRHI::RHIinstance->buffer->GetSize();

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(Context->device, 1, &descriptorWrite, 0, nullptr);
	}
#endif
	return descriptorSets[0];
}
