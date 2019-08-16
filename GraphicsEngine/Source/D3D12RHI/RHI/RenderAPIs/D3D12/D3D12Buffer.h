#pragma once
#include "D3D12RHI.h"
class D3D12Buffer : public RHIBuffer
{
public:
	D3D12Buffer(ERHIBufferType::Type type, DeviceContext* Device = nullptr);
	virtual ~D3D12Buffer();
	virtual void CreateConstantBuffer(int StructSize, int Elementcount, bool ReplicateToAllDevices = false) override;
	virtual void CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype = EBufferAccessType::Static) override;
	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void UpdateIndexBuffer(void* data, size_t length) override;
	virtual void UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state) override;
	virtual void CreateIndexBuffer(int Stride, int ByteSize) override;
	virtual void CreateBuffer(RHIBufferDesc desc) override;
	virtual void UpdateVertexBuffer(void* data, size_t length) override;
	virtual void BindBufferReadOnly(RHICommandList* list, int RSSlot)override;
	virtual void SetBufferState(class RHICommandList* list, EBufferResourceState::Type State) override;
	bool CheckDevice(int index);
	void EnsureResouceInFinalState(ID3D12GraphicsCommandList* list);
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	void SetConstantBufferView(int offset, ID3D12GraphicsCommandList * list, int Slot, bool IsCompute, int Deviceindex);
	GPUResource* GetResource();
	DescriptorGroup* GetDescriptor();
	void SetupBufferSRV();
protected:
	void UpdateData(void * data, size_t length, D3D12_RESOURCE_STATES EndState);
	void Release() override;
	
	void CreateUAV();
	friend class D3D12RHIUAV;
private:
	void MapBuffer(void** Data);
	void UnMap();
	void CreateStaticBuffer(int ByteSize);
	void CreateDynamicBuffer(int ByteSize);

	D3D12CBV* CBV[MAX_GPU_DEVICE_COUNT][RHI::CPUFrameCount] = { nullptr };
	EBufferAccessType::Type BufferAccesstype;
	GPUResource * m_UploadBuffer = nullptr;
	GPUResource* m_DataBuffer = nullptr;
	int ElementCount = 0;
	int ElementSize = 0;
	bool UploadComplete = false;
	bool CrossDevice = false;
	D3D12DeviceContext* Device = nullptr;
	DescriptorGroup* SRVDesc = nullptr;
	D3D12_RESOURCE_STATES PostUploadState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
};
CreateChecker(D3D12Buffer);
