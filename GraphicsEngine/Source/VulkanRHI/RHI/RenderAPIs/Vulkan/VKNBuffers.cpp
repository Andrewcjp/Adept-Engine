#include "VKNBuffers.h"
#include "Core/Platform/PlatformCore.h"
#include "VKNRHI.h"
#include "VKNDeviceContext.h"
#include "Descriptor.h"
#include "VKNHelpers.h"
#undef max

void VKNBuffer::CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype/* = EBufferAccessType::Static*/)
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

	if (vkCreateBuffer(VKNRHI::GetVDefaultDevice()->device, &bufferInfo, nullptr, &vertexbuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}
	//#VK upload to local segment
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(VKNRHI::GetVDefaultDevice()->device, vertexbuffer, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VKNHelpers::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(VKNRHI::GetVDefaultDevice()->device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}
	vkBindBufferMemory(VKNRHI::GetVDefaultDevice()->device, vertexbuffer, vertexBufferMemory, 0);
	ensure(vertexBufferMemory);
}

void VKNBuffer::UpdateVertexBuffer(void * data, size_t length)
{
	VertexCount = length /*/ StructSize*/;
	void* GPUdata;
	vkMapMemory(VKNRHI::GetVDefaultDevice()->device, vertexBufferMemory, 0, TotalByteSize, 0, &GPUdata);
	if (Desc.Accesstype == EBufferAccessType::Dynamic)
	{
		memcpy(GPUdata, data, length);
	}
	else
	{
		VertexCount = length / StructSize;
		memcpy(GPUdata, data, TotalByteSize);
	}
	vkUnmapMemory(VKNRHI::GetVDefaultDevice()->device, vertexBufferMemory);
}

void VKNBuffer::UpdateIndexBuffer(void * data, size_t length)
{
	VertexCount = length;
	void* GPUdata;
	vkMapMemory(VKNRHI::GetVDefaultDevice()->device, vertexBufferMemory, 0, TotalByteSize, 0, &GPUdata);
	memcpy(GPUdata, data, TotalByteSize);
	vkUnmapMemory(VKNRHI::GetVDefaultDevice()->device, vertexBufferMemory);
}

void VKNBuffer::BindBufferReadOnly(RHICommandList * list, int RSSlot)
{
	list->SetConstantBufferView(this, 0, RSSlot);
}

void VKNBuffer::SetBufferState(RHICommandList * list, EBufferResourceState::Type State)
{}


void VKNBuffer::CreateIndexBuffer(int Stride, int ByteSize)
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

	if (vkCreateBuffer(VKNRHI::GetVDefaultDevice()->device, &bufferInfo, nullptr, &vertexbuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}
	//#VK upload to local segment
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(VKNRHI::GetVDefaultDevice()->device, vertexbuffer, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = VKNHelpers::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(VKNRHI::GetVDefaultDevice()->device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}
	vkBindBufferMemory(VKNRHI::GetVDefaultDevice()->device, vertexbuffer, vertexBufferMemory, 0);
}



void VKNBuffer::CreateConstantBuffer(int iStructSize, int Elementcount, bool ReplicateToAllDevices/* = false*/)
{
	StructSize = VKNHelpers::Align(iStructSize);
	TotalByteSize = StructSize * Elementcount;

	VKNHelpers::createBuffer(StructSize*Elementcount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexbuffer, vertexBufferMemory);
	ensure(vertexBufferMemory);
}

void VKNBuffer::UpdateConstantBuffer(void * data, int offset)
{
	//#VK: Bounds check
	void* GPUdata;
	vkMapMemory(VKNRHI::GetVDefaultDevice()->device, vertexBufferMemory, StructSize*offset, StructSize, 0, &GPUdata);
	memcpy(GPUdata, data, StructSize);
	vkUnmapMemory(VKNRHI::GetVDefaultDevice()->device, vertexBufferMemory);
}

void VKNBuffer::UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state)
{
	void* GPUdata;
	vkMapMemory(VKNRHI::GetVDefaultDevice()->device, vertexBufferMemory, 0, TotalByteSize, 0, &GPUdata);
	memcpy(GPUdata, data, length);
	vkUnmapMemory(VKNRHI::GetVDefaultDevice()->device, vertexBufferMemory);
}

Descriptor VKNBuffer::GetDescriptor(int slot, int offset)
{
	Descriptor D = Descriptor(EDescriptorType::CBV);
	D.Buffer = this;
	D.bindpoint = slot;
	D.Offset = VKNHelpers::Align(StructSize* offset);
	return D;
}

void VKNBuffer::CreateBuffer(RHIBufferDesc desc)
{
	StructSize = VKNHelpers::Align(desc.Stride);
	TotalByteSize = StructSize * desc.ElementCount;

	VKNHelpers::createBuffer(StructSize*desc.ElementCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexbuffer, vertexBufferMemory);
	ensure(vertexBufferMemory);
}
