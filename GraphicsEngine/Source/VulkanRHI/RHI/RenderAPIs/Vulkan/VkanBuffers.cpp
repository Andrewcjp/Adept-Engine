#include "VkanBuffers.h"
#include "Core/Platform/PlatformCore.h"
#include "VKanRHI.h"
#include "VkanDeviceContext.h"
#include "Descriptor.h"
#include "VkanHelpers.h"
#if BUILD_VULKAN



void VKanBuffer::CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype/* = EBufferAccessType::Static*/)
{
	TotalByteSize = ByteSize;
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = Stride;
	StructSize = Stride;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	Desc.Accesstype = Accesstype;
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = ByteSize;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VKanRHI::GetVDefaultDevice()->device, &bufferInfo, nullptr, &vertexbuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}
	//#VK upload to local segment
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(VKanRHI::GetVDefaultDevice()->device, vertexbuffer, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VkanHelpers::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(VKanRHI::GetVDefaultDevice()->device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}
	vkBindBufferMemory(VKanRHI::GetVDefaultDevice()->device, vertexbuffer, vertexBufferMemory, 0);
	ensure(vertexBufferMemory);
}

void VKanBuffer::UpdateVertexBuffer(void * data, size_t length)
{
	VertexCount = length /*/ StructSize*/;
	void* GPUdata;
	vkMapMemory(VKanRHI::GetVDefaultDevice()->device, vertexBufferMemory, 0, TotalByteSize, 0, &GPUdata);
	if (Desc.Accesstype == EBufferAccessType::Dynamic)
	{
		memcpy(GPUdata, data, length);
	}
	else
	{
		memcpy(GPUdata, data, TotalByteSize);
	}
	vkUnmapMemory(VKanRHI::GetVDefaultDevice()->device, vertexBufferMemory);
}

void VKanBuffer::UpdateIndexBuffer(void * data, size_t length)
{
	VertexCount = length;
	void* GPUdata;
	vkMapMemory(VKanRHI::GetVDefaultDevice()->device, vertexBufferMemory, 0, TotalByteSize, 0, &GPUdata);
	memcpy(GPUdata, data, TotalByteSize);
	vkUnmapMemory(VKanRHI::GetVDefaultDevice()->device, vertexBufferMemory);
}

void VKanBuffer::BindBufferReadOnly(RHICommandList * list, int RSSlot)
{}

void VKanBuffer::SetBufferState(RHICommandList * list, EBufferResourceState::Type State)
{}


void VKanBuffer::CreateIndexBuffer(int Stride, int ByteSize)
{
	TotalByteSize = ByteSize;
	//VkVertexInputBindingDescription bindingDescription = {};
	//bindingDescription.binding = 0;
	//bindingDescription.stride = Stride;
	//bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = ByteSize;
	bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VKanRHI::GetVDefaultDevice()->device, &bufferInfo, nullptr, &vertexbuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}
	//#VK upload to local segment
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(VKanRHI::GetVDefaultDevice()->device, vertexbuffer, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VkanHelpers::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(VKanRHI::GetVDefaultDevice()->device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}
	vkBindBufferMemory(VKanRHI::GetVDefaultDevice()->device, vertexbuffer, vertexBufferMemory, 0);
}



void VKanBuffer::CreateConstantBuffer(int iStructSize, int Elementcount, bool ReplicateToAllDevices/* = false*/)
{
	TotalByteSize = iStructSize * Elementcount;
	StructSize = iStructSize;
	VkanHelpers::createBuffer(iStructSize*Elementcount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexbuffer, vertexBufferMemory);
	ensure(vertexBufferMemory);
}

void VKanBuffer::UpdateConstantBuffer(void * data, int offset)
{
	void* GPUdata;
	vkMapMemory(VKanRHI::GetVDefaultDevice()->device, vertexBufferMemory, 0, TotalByteSize, 0, &GPUdata);
	memcpy(GPUdata, data, TotalByteSize);
	vkUnmapMemory(VKanRHI::GetVDefaultDevice()->device, vertexBufferMemory);
}

void VKanBuffer::UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state)
{

}

Descriptor VKanBuffer::GetDescriptor(int slot, int offset)
{
	Descriptor D = Descriptor(EDescriptorType::CBV);
	D.Buffer = this;
	D.bindpoint = slot;
	D.Offset = offset;
	return D;
}

void VKanBuffer::CreateBuffer(RHIBufferDesc desc)
{

}
#endif