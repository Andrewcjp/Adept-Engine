#pragma once
#include "RHI\RHITypes.h"
struct EDescriptorType
{
	enum Type
	{
		CBV,
		SRV,
		UAV,
		Limit
	};
};
class DescriptorHeap;
class Descriptor : public IRHIResourse
{
public:
	Descriptor();
	~Descriptor();
	void Init(D3D12_DESCRIPTOR_HEAP_TYPE Type, DescriptorHeap* heap, int size);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUAddress(int index = 0);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUAddress(int index = 0);
	int indexInHeap = 0;
	D3D12_DESCRIPTOR_HEAP_TYPE GetType();
	int GetSize();
	DescriptorHeap* Owner;
	void Recreate();
	void CreateShaderResourceView(ID3D12Resource  *pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC *pDesc, int offset = 0);
	void CreateUnorderedAccessView(ID3D12Resource *pResource, ID3D12Resource *pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC *pDesc, int offset = 0);

	virtual void Release() override;

	//create shader view
private:
	int DescriptorCount = 1;
	//TODO: descriptor in multiple heaps
	EDescriptorType::Type DescriptorType = EDescriptorType::Limit;
	D3D12_DESCRIPTOR_HEAP_TYPE Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	ID3D12Resource* TargetResource = nullptr;
	ID3D12Resource* UAVCounterResource = nullptr;
	int OffsetInHeap = 0;

};

