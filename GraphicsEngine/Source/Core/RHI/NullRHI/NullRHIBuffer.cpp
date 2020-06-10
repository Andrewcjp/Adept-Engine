#include "NullRHIBuffer.h"

void NullRHIBuffer::CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype /*= EBufferAccessType::Static*/)
{

}

void NullRHIBuffer::CreateBuffer(RHIBufferDesc Desc)
{

}

void NullRHIBuffer::CreateIndexBuffer(int Stride, int ByteSize)
{

}

void NullRHIBuffer::CreateConstantBuffer(int StructSize, int Elementcount, bool ReplicateToAllDevices /*= false*/)
{

}

void NullRHIBuffer::UpdateConstantBuffer(void * data, int offset /*= 0*/)
{

}

void NullRHIBuffer::UpdateVertexBuffer(void* data, size_t length, int VertexCount /*= -1*/)
{

}

void NullRHIBuffer::UpdateIndexBuffer(void* data, size_t length)
{

}

void NullRHIBuffer::BindBufferReadOnly(class RHICommandList* list, int RSSlot)
{

}

void NullRHIBuffer::SetBufferState(class RHICommandList* list, EBufferResourceState::Type State)
{

}

void NullRHIBuffer::SetResourceState(RHICommandList* list, EResourceState::Type State)
{

}

void NullRHIBuffer::UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state)
{

}

void* NullRHIBuffer::MapReadBack()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void NullRHIBuffer::UnMap()
{
	throw std::logic_error("The method or operation is not implemented.");
}
