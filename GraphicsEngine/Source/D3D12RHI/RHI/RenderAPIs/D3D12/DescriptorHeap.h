#pragma once
class DeviceContext;
class D3D12CommandList;
class Descriptor;
class DescriptorHeap
{
public:
	enum EGPUMemoryPriority
	{
		Critical,
		Scaled,
		RenderBuffers,
		Streaming
	};
	DescriptorHeap(DeviceContext* inDevice, int Num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	~DescriptorHeap();
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuAddress(int index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUAddress(int index);
	void SetName(LPCWSTR name);
	
	void BindHeap(D3D12CommandList * list);
	void Release();
	void SetPriority(EGPUMemoryPriority NewPriority);
	ID3D12DescriptorHeap* GetHeap()
	{
		return mHeap;
	};
	std::string GetDebugName()
	{
		return "DescriptorHeap";
	}
	void BindHeap_Old(ID3D12GraphicsCommandList* list);
	void AddDescriptor(Descriptor* desc);
	int GetNumberOfDescriptors();
	int GetMaxSize();
	int GetNextFreeIndex();
private:
	std::vector<Descriptor*> ContainedDescriptors;
	ID3D12DescriptorHeap * mHeap = nullptr;
	class D3D12DeviceContext* Device = nullptr;
	int DescriptorOffsetSize = 0;
	EGPUMemoryPriority Priority = EGPUMemoryPriority::Critical;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	int DescriptorCount = 0;
};

