#include "VkanBuffers.h"
#include "Core/Platform/PlatformCore.h"
#include "VKanRHI.h"
#include "VkanDeviceContext.h"
#if BUILD_VULKAN

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(VKanRHI::GetVDefaultDevice()->physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}
void VKanBuffer::CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype/* = EBufferAccessType::Static*/)
{
	TotalByteSize = ByteSize;
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = Stride;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = ByteSize;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VKanRHI::GetVDefaultDevice()->device, &bufferInfo, nullptr, &vertexbuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(VKanRHI::GetVDefaultDevice()->device, vertexbuffer, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(VKanRHI::GetVDefaultDevice()->device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}
	vkBindBufferMemory(VKanRHI::GetVDefaultDevice()->device, vertexbuffer, vertexBufferMemory, 0);
}

void VKanBuffer::UpdateVertexBuffer(void * data, size_t length)
{
	void* GPUdata;
	vkMapMemory(VKanRHI::GetVDefaultDevice()->device, vertexBufferMemory, 0, TotalByteSize, 0, &GPUdata);
	memcpy(GPUdata, data, length);
	vkUnmapMemory(VKanRHI::GetVDefaultDevice()->device, vertexBufferMemory);
}




void VKanBuffer::UpdateIndexBuffer(void * data, size_t length)
{}

void VKanBuffer::BindBufferReadOnly(RHICommandList * list, int RSSlot)
{}

void VKanBuffer::SetBufferState(RHICommandList * list, EBufferResourceState::Type State)
{}





void VKanBuffer::CreateIndexBuffer(int Stride, int ByteSize)
{}
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(VKanRHI::GetVDefaultDevice()->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(VKanRHI::GetVDefaultDevice()->device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(VKanRHI::GetVDefaultDevice()->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(VKanRHI::GetVDefaultDevice()->device, buffer, bufferMemory, 0);
}
void VKanBuffer::CreateConstantBuffer(int iStructSize, int Elementcount, bool ReplicateToAllDevices/* = false*/)
{
	TotalByteSize = iStructSize * Elementcount;
	StructSize = iStructSize;
	createBuffer(iStructSize*Elementcount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexbuffer, vertexBufferMemory);
	ensure(vertexBufferMemory);

}
void VKanBuffer::UpdateConstantBuffer(void * data, int offset)
{
	void* GPUdata;
	vkMapMemory(VKanRHI::GetVDefaultDevice()->device, vertexBufferMemory, 0, TotalByteSize, 0, &GPUdata);
	memcpy(GPUdata, data, StructSize);
	vkUnmapMemory(VKanRHI::GetVDefaultDevice()->device, vertexBufferMemory);
}
void VKanBuffer::UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state)
{

}

void VKanBuffer::CreateBuffer(RHIBufferDesc desc)
{

}
#endif