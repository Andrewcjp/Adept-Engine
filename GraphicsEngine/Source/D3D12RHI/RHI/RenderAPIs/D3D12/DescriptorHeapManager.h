#pragma once

class Descriptor;
class DescriptorHeap;
class D3D12CommandList;
class D3D12DeviceContext;
class DescriptorGroup;
class DescriptorHeapManager
{
public:
	DescriptorHeapManager(D3D12DeviceContext* Device);
	void AllocateMainHeap(int size);
	~DescriptorHeapManager();
	Descriptor* AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, int size = 1);
	DescriptorGroup* AllocateDescriptorGroup(D3D12_DESCRIPTOR_HEAP_TYPE type, int size = 1);

	void CheckAndRealloc(int size);

	DescriptorHeap* GetMainHeap();
	void BindHeap(D3D12CommandList* list);
	static void Reallocate(DescriptorHeap** TargetHeat, int newsize);
	void EndOfFrame();

private:
	D3D12DeviceContext* Device = nullptr;
	DescriptorHeap* MainHeap[RHI::CPUFrameCount] = { nullptr,nullptr };
	DescriptorHeap* SamplerHeap = nullptr;
	std::vector<DescriptorGroup*> Groups;
};

