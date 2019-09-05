#include "VulkanRHIPCH.h"
#include "VKNDescriptorPool.h"
#include "VKNRHI.h"
#include "VKNDeviceContext.h"
#include "VKNCommandlist.h"
#include "VKNPipeLineStateObject.h"
#include "VKNBuffers.h"
#include "Descriptor.h"
#include "VKNTexture.h"
#include "VKNShader.h"
#include "Core/Performance/PerfManager.h"
#include "RHI/RHIRootSigniture.h"
#include "VKNFramebuffer.h"


VKNDescriptorPool::VKNDescriptorPool(VKNDeviceContext* Con)
{
	Context = Con;
	Init();
}


VKNDescriptorPool::~VKNDescriptorPool()
{}

void VKNDescriptorPool::Init()
{
	createDescriptorPool();
}

void VKNDescriptorPool::ResetAllocations()
{
	vkResetDescriptorPool(Context->device, descriptorPool, 0);
}

void VKNDescriptorPool::AllocateAndBind(VKNCommandlist * List)
{
	SCOPE_CYCLE_COUNTER_GROUP("Descriptor Bind", "RHI");
	VkDescriptorSet Set = AllocateSet(List);
	vkCmdBindDescriptorSets(List->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, List->CurrentPso->PipelineLayout, 0, 1, &Set, 0, nullptr);
}
#define STATIC_SAMPLER 1
VkDescriptorSet VKNDescriptorPool::AllocateSet(VKNCommandlist * list)
{
	VkDescriptorSet Set = createDescriptorSets(list->CurrentPso->descriptorSetLayout, 1);
	std::vector<VkWriteDescriptorSet> WriteData;
	for (int i = 0; i < list->Rootsig.GetNumBinds(); i++)
	{
		const RSBind* Desc = list->Rootsig.GetBind(i);
		if (Desc->BindType == ERSBindType::Limit)
		{
			//__debugbreak();
			continue;
		}
		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = Set;
		descriptorWrite.dstBinding = Desc->BindParm->RegisterSlot;//todo check this
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorCount = Desc->BindParm->NumDescriptors;
		if (Desc->BindType == ERSBindType::CBV)
		{
			VkDescriptorBufferInfo* bufferInfo = new VkDescriptorBufferInfo();
			if (Desc->BufferTarget != nullptr)
			{
				bufferInfo->buffer = VKNRHI::VKConv(Desc->BufferTarget)->vertexbuffer;
			}
			else
			{
				bufferInfo->buffer = VKNRHI::VKConv(VKNRHI::RHIinstance->buffer)->vertexbuffer;
			}
			bufferInfo->offset = Desc->Offset;

			bufferInfo->range = VK_WHOLE_SIZE;//Desc->Buffer->GetSize();
			descriptorWrite.pBufferInfo = bufferInfo;
			if (Desc->BindParm->Type == ShaderParamType::CBV)
			{
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			}
			else
			{
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			}
		}
		else if (Desc->BindParm->Type == ShaderParamType::SRV)
		{
			VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
			imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (Desc->Framebuffer != nullptr)
			{
				imageInfo->imageView = VKNRHI::VKConv(Desc->Framebuffer)->RTImageView[Desc->Offset];
			}
			else if (Desc->Texture != nullptr)
			{
				imageInfo->imageView = VKNRHI::VKConv(Desc->Texture.Get())->textureImageView;
			}
			else/* if (imageInfo->imageView == VK_NULL_HANDLE)*/
			{
				//LogEnsure(imageInfo->imageView == VK_NULL_HANDLE);
				imageInfo->imageView = VKNRHI::RHIinstance->T->textureImageView;
				continue;
			}
			imageInfo->sampler = list->CurrentPso->textureSampler;
			descriptorWrite.dstBinding += VKNShader::GetBindingOffset(ShaderParamType::SRV);
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

void VKNDescriptorPool::createDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> Sizes;
	int MaxCount = 10000;
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
	poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
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

VkDescriptorSet VKNDescriptorPool::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, int count)
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
