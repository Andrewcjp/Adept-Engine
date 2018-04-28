#pragma once
#include "D3D12RHI.h"
class D3D12Heap
{
public:
	D3D12Heap();
	~D3D12Heap();
	enum HeapType
	{
		SRVCBV,
		Sampler,
		RTV,
		DSV,
		Limit
	};
	//a static heap is One That Not change during a frame
	void CreateStaticHeap(int NumDescriptors, HeapType type);
	//A Heap that could changed multiple times per frame
	void CreateDynamicHeap(int NumDescriptors, HeapType type);
	bool GetIsDynamic() { return IsDynamic; }
private:
	bool IsDynamic = false;
	ID3D12DescriptorHeap * m_Heap;
	HeapType CurrentType = HeapType::Limit;
	//First CPU descriptor handle in this allocation
	D3D12_CPU_DESCRIPTOR_HANDLE m_FirstCpuHandle = { 0 };

	// First GPU descriptor handle in this allocation
	D3D12_GPU_DESCRIPTOR_HANDLE m_FirstGpuHandle = { 0 };
};

