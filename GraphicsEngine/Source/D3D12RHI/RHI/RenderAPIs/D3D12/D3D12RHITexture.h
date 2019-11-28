#pragma once
#include "RHI\RHITexture.h"

class D3D12DeviceContext;
class DXDescriptor;
class DescriptorHeap;
struct RHIViewDesc;
class D3D12RHITexture :
	public RHITexture
{
public:
	D3D12RHITexture();
	virtual ~D3D12RHITexture();
	void Release() override;
	size_t GetSizeOnGPU() override;
	DXDescriptor * GetDescriptor(const RHIViewDesc & desc, DescriptorHeap * heap);
	void Create(const RHITextureDesc2& Desc, DeviceContext* Context) override;
	GPUResource* GetResource()const;
	void WriteToDescriptor(DXDescriptor* Descriptor, const RHIViewDesc& desc);
private:
	GPUResource* Resource = nullptr;
	D3D12DeviceContext* DContext = nullptr;
	CD3DX12_RESOURCE_DESC ResourceDesc = CD3DX12_RESOURCE_DESC();
};

