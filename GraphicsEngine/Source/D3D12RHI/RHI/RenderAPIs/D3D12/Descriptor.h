#pragma once

class DescriptorHeap;
class Descriptor
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
	//create shader view
private:
	int DescriptorCount = 1;
	//TODO: descriptor in multiple heaps

	D3D12_DESCRIPTOR_HEAP_TYPE Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	D3D12_SHADER_RESOURCE_VIEW_DESC ViewDesc = {};
	ID3D12Resource* TargetResource = nullptr;
	int SRVOffset = 0;

};

