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
#include "VKNHelpers.h"
#include "Rendering/Core/Defaults.h"


VKNDescriptorPool::VKNDescriptorPool(VKNDeviceContext* Con)
{
	Context = Con;
	Init();
#if USERV
	CopyData.Allocate(20);
	WriteData.Allocate(20);
#endif
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
	BufferInfoAlloc.Reset();
	ImageInfoAlloc.Reset();
	LastUsedSet = NULL;
}

void VKNDescriptorPool::AllocateAndBind(VKNCommandlist * RESTRICT List)
{
	//SCOPE_CYCLE_COUNTER_GROUP("Descriptor Bind", "RHI");
	VkDescriptorSet Set = AllocateSet(List);
	vkCmdBindDescriptorSets(List->CommandBuffer, List->GetBindPoint(), List->CurrentPso->PipelineLayout, 0, 1, &Set, 0, nullptr);
	
}
#define STATIC_SAMPLER 1
VkDescriptorSet VKNDescriptorPool::AllocateSet(VKNCommandlist * RESTRICT list)
{
	BufferInfoAlloc.Reset();
	ImageInfoAlloc.Reset();
	VkDescriptorSet Set = createDescriptorSets(&list->CurrentPso->descriptorSetLayout, 1);
	WriteData.clear();
	CopyData.clear();
	list->Rootsig.ValidateAllBound();
	for (int i = 0; i < list->Rootsig.GetNumBinds(); i++)
	{
		const RSBind* Desc = list->Rootsig.GetBind(i);
		if (Desc->BindType == ERSBindType::Limit)
		{
			//__debugbreak();
			continue;
		}
		if (!Desc->HasChanged && LastUsedSet != NULL)
		{
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
			VkDescriptorBufferInfo* bufferInfo = BufferInfoAlloc.Allocate();/*new VkDescriptorBufferInfo()*/
			if (Desc->BufferTarget != nullptr)
			{
				bufferInfo->buffer = VKNRHI::VKConv(Desc->BufferTarget)->vertexbuffer;
				bufferInfo->offset = VKNHelpers::Align(VKNRHI::VKConv(Desc->BufferTarget)->StructSize* Desc->Offset);
				bufferInfo->range = VKNHelpers::Align(Desc->BufferTarget->GetSize(Desc->Offset));
				if (bufferInfo->range == 0)
				{
					LogEnsureMsgf(bufferInfo->range == 0);
					bufferInfo->offset = 0;
					bufferInfo->range = Desc->BufferTarget->GetSize();

				}
			}
			else
			{
				bufferInfo->buffer = VKNRHI::VKConv(VKNRHI::RHIinstance->buffer)->vertexbuffer;
				bufferInfo->range = 256;
				bufferInfo->offset = 0;
			}
			descriptorWrite.dstBinding += VKNShader::GetBindingOffset(ShaderParamType::CBV);
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
			VkDescriptorImageInfo* imageInfo = ImageInfoAlloc.Allocate();
			imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (Desc->Framebuffer != nullptr)
			{
				if (Desc->Framebuffer->GetDescription().RenderTargetCount == 0)
				{
					imageInfo->imageView = VKNRHI::VKConv(Desc->Framebuffer)->depthImageView;
				}
				else
				{
					imageInfo->imageView = VKNRHI::VKConv(Desc->Framebuffer)->RTImageView[Desc->Offset];
				}
			}
			else if (Desc->Texture != nullptr)
			{
				imageInfo->imageView = VKNRHI::VKConv(Desc->Texture.Get())->textureImageView;
			}
			else
			{
				imageInfo->imageView = VKNRHI::VKConv(Defaults::GetDefaultTexture().Get())->textureImageView;
			}
			ensure(imageInfo->imageView != 0);
			imageInfo->sampler = list->CurrentPso->textureSampler;
			descriptorWrite.dstBinding += VKNShader::GetBindingOffset(ShaderParamType::SRV);
			descriptorWrite.pImageInfo = imageInfo;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		}
		else if (Desc->BindParm->Type == ShaderParamType::UAV)
		{
			VkDescriptorBufferInfo* bufferInfo = BufferInfoAlloc.Allocate();
			if (Desc->UAVTarget != nullptr)
			{
				bufferInfo->buffer = *VKNRHI::VKConv(Desc->UAVTarget)->TargetBuffer;
				bufferInfo->offset = 0;// VKNHelpers::Align(VKNRHI::VKConv(Desc->BufferTarget)->StructSize* Desc->Offset);
				bufferInfo->range = VKNRHI::VKConv(Desc->UAVTarget)->TargetSize;//VKNHelpers::Align(Desc->BufferTarget->GetSize(Desc->Offset));
				/*if (bufferInfo->range == 0)
				{
					LogEnsureMsgf(bufferInfo->range == 0);
					bufferInfo->offset = 0;
					bufferInfo->range = Desc->BufferTarget->GetSize();
				}*/
			}
			else 
			{
				bufferInfo->buffer = VKNRHI::VKConv(VKNRHI::RHIinstance->buffer)->vertexbuffer;
				bufferInfo->range = 256;
				bufferInfo->offset = 0;
			}
			descriptorWrite.pBufferInfo = bufferInfo;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}
		WriteData.push_back(descriptorWrite);
	}
	for (int i = 0; i < list->Rootsig.GetNumBinds(); i++)
	{
		const RSBind* Desc = list->Rootsig.GetBind(i);
		if (Desc->BindType == ERSBindType::Limit)
		{
			//__debugbreak();
			continue;
		}
		if (LastUsedSet == NULL)
		{
			continue;
		}
		if (Desc->HasChanged)
		{
			continue;
		}
		VkCopyDescriptorSet descriptorCopy = {};
		descriptorCopy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
		descriptorCopy.dstSet = Set;
		descriptorCopy.dstBinding = Desc->BindParm->RegisterSlot + VKNShader::GetBindingOffset(Desc->BindParm->Type);//todo check this
		descriptorCopy.dstArrayElement = 0;
		descriptorCopy.descriptorCount = 1;// Desc->BindParm->NumDescriptors;
		descriptorCopy.srcBinding = Desc->BindParm->RegisterSlot + VKNShader::GetBindingOffset(Desc->BindParm->Type);
		descriptorCopy.srcSet = LastUsedSet;
		descriptorCopy.dstSet = Set;
		CopyData.push_back(descriptorCopy);
	}
	vkUpdateDescriptorSets(Context->device, WriteData.size(), WriteData.data(), CopyData.size(), CopyData.data());
	list->Rootsig.SetUpdated();
	LastUsedSet = Set;
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

VkDescriptorSet VKNDescriptorPool::createDescriptorSets(const VkDescriptorSetLayout* RESTRICT descriptorSetLayout, int count)
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = count;
	allocInfo.pSetLayouts = descriptorSetLayout;
	VkDescriptorSet Set;
	if (vkAllocateDescriptorSets(Context->device, &allocInfo, &Set) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	return Set;
}
