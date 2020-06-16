#include "RHIBufferGroup.h"
#include "RHI/DeviceContext.h"


RHIBufferGroup::RHIBufferGroup()
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		Buffers[i] = RHI::CreateRHIBuffer(ERHIBufferType::Constant, RHI::GetDeviceContext(i));
	}
}

RHIBufferGroup::~RHIBufferGroup()
{}

RHIBuffer* RHIBufferGroup::Get(DeviceContext* con)
{
	return Buffers[con->GetDeviceIndex()];
}

RHIBuffer * RHIBufferGroup::Get(RHICommandList * list)
{
	return Buffers[list->GetDeviceIndex()];
}

RHIBuffer * RHIBufferGroup::Get(int index)
{
	return  Buffers[index];
}

void RHIBufferGroup::CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype)
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		Buffers[i]->CreateVertexBuffer(Stride, ByteSize, Accesstype);
	}
}

void RHIBufferGroup::CreateBuffer(RHIBufferDesc Desc)
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		Buffers[i]->CreateBuffer(Desc);
	}
}

void RHIBufferGroup::CreateIndexBuffer(int Stride, int ByteSize)
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		Buffers[i]->CreateIndexBuffer(Stride, ByteSize);
	}
}

void RHIBufferGroup::CreateConstantBuffer(int StructSize, int Elementcount, bool ReplicateToAllDevices)
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		Buffers[i]->CreateConstantBuffer(StructSize, Elementcount);
	}
}

void RHIBufferGroup::UpdateConstantBuffer(void * data, int offset)
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		Buffers[i]->UpdateConstantBuffer(data, offset);
	}
}

void RHIBufferGroup::UpdateVertexBuffer(void * data, size_t length, int VertexCount)
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		Buffers[i]->UpdateVertexBuffer(data, length, VertexCount);
	}
}

void RHIBufferGroup::UpdateIndexBuffer(void * data, size_t length)
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		Buffers[i]->UpdateIndexBuffer(data, length);
	}
}

void RHIBufferGroup::SetBufferState(RHICommandList * list, EBufferResourceState::Type State)
{}

void RHIBufferGroup::UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state)
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		Buffers[i]->UpdateBufferData(data, length, state);
	}
}
