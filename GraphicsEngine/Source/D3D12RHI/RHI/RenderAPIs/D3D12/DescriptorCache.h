#pragma once

class DXDescriptor;
class DescriptorHeap;
struct RSBind;
class D3D12DeviceContext;
class DescriptorCache
{
public:
	DescriptorCache(D3D12DeviceContext* con);
	void OnHeapClear();
	~DescriptorCache();

	uint64 GetHash(const RSBind * bind);

	bool FindInCache(uint64 hash, DXDescriptor ** desc, ERSBindType::Type type);

	DXDescriptor* CopyToCurrentHeap(DXDescriptor * d, bool CouldbeReused  = true);

	bool ShouldCache(const RSBind * bind);

	DXDescriptor * Create(const RSBind * bind, DescriptorHeap * heap);

	DXDescriptor* GetOrCreate(const RSBind* bind);
private:
	DescriptorHeap* CacheHeap = nullptr;
	std::map<uint64, DXDescriptor*> TextureMap[ERSBindType::Limit];
	D3D12DeviceContext* Device = nullptr;
	const char* TimerName = "Cpy Desc";
	const char* ReuseTimer = "Reuse Desc";
	std::map<DXDescriptor*, DXDescriptor*> DescriptorsInHeap;
};

