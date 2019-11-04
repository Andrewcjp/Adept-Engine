#pragma once
#include "RHI\RHIRootSigniture.h"

class DXDescriptor;
class DescriptorHeap;
struct RSBind;
class D3D12DeviceContext;
class DescriptorCache
{
public:
	DescriptorCache(D3D12DeviceContext* con);
	void OnHeapClear();
	void Invalidate();
	void RemoveInvalidCaches();
	~DescriptorCache();

	uint64 GetHash(const RSBind * bind);

	bool FindInCache(uint64 hash, DXDescriptor ** desc, ERSBindType::Type type);

	DXDescriptor* CopyToCurrentHeap(DXDescriptor * d, bool CouldbeReused  = true);

	bool ShouldCache(const RSBind * bind);

	DXDescriptor * Create(const RSBind * bind, DescriptorHeap * heap);

	DXDescriptor* GetOrCreate(const RSBind* bind);
private:
	struct DescriptorRef
	{
		int LastUsedFrame = 0;
		DXDescriptor* desc = nullptr;
	};
	DescriptorHeap* CacheHeap = nullptr;
	std::map<uint64, DescriptorRef> DescriptorMap[ERSBindType::Limit];
	D3D12DeviceContext* Device = nullptr;
	const char* TimerName = "Cpy Desc";
	const char* ReuseTimer = "Reuse Desc";
	const char* MissTimer = "Desc cache miss";
	std::map<DXDescriptor*, DXDescriptor*> DescriptorsInHeap;
};

