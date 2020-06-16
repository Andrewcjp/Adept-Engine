#pragma once
#include "RHICommandList.h"

class RHIBuffer;
class RHIBufferGroup : public IRHIResourse
{
public:
	RHIBufferGroup();
	~RHIBufferGroup();

	RHI_API RHIBuffer* Get(DeviceContext* con);
	RHI_API RHIBuffer* Get(RHICommandList* list);
	RHI_API RHIBuffer* Get(int index);
	void CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype = EBufferAccessType::Static);
	void CreateBuffer(RHIBufferDesc Desc);
	void CreateIndexBuffer(int Stride, int ByteSize);
	void CreateConstantBuffer(int StructSize, int Elementcount, bool ReplicateToAllDevices = false);
	void UpdateConstantBuffer(void * data, int offset = 0);
	void UpdateVertexBuffer(void* data, size_t length, int VertexCount = -1);
	void UpdateIndexBuffer(void* data, size_t length);
	void SetBufferState(class RHICommandList* list, EBufferResourceState::Type State);
	void UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state);
	
private:
	RHIBuffer* Buffers[MAX_GPU_DEVICE_COUNT];
};

