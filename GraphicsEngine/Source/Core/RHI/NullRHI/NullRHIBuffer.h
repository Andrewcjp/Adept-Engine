#pragma once
#include "RHI/RHICommandList.h"
class NullRHIBuffer: public RHIBuffer
{

public:
	NullRHIBuffer() :RHIBuffer(ERHIBufferType::Vertex)
	{

	}
	RHI_API RHI_VIRTUAL void CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype = EBufferAccessType::Static) override;
	RHI_API RHI_VIRTUAL void CreateBuffer(RHIBufferDesc Desc) override;
	RHI_API RHI_VIRTUAL void CreateIndexBuffer(int Stride, int ByteSize) override;
	RHI_API RHI_VIRTUAL void CreateConstantBuffer(int StructSize, int Elementcount, bool ReplicateToAllDevices = false) override;
	RHI_API RHI_VIRTUAL void UpdateConstantBuffer(void * data, int offset = 0) override;
	RHI_API RHI_VIRTUAL void UpdateVertexBuffer(void* data, size_t length, int VertexCount = -1) override;
	RHI_API RHI_VIRTUAL void UpdateIndexBuffer(void* data, size_t length) override;
	RHI_API RHI_VIRTUAL void BindBufferReadOnly(class RHICommandList* list, int RSSlot) override;
	RHI_API RHI_VIRTUAL void SetBufferState(class RHICommandList* list, EBufferResourceState::Type State) override;
	RHI_API RHI_VIRTUAL void SetResourceState(RHICommandList* list, EResourceState::Type State) override;
	RHI_API RHI_VIRTUAL void UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state) override;
	RHI_API RHI_VIRTUAL void* MapReadBack() override;


	RHI_API RHI_VIRTUAL void UnMap() override;

};

