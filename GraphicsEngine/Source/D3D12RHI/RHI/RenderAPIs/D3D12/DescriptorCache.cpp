#include "DescriptorCache.h"
#include "RHI\RHIRootSigniture.h"
#include "DXDescriptor.h"
#include "D3D12CommandList.h"
#include "DescriptorHeapManager.h"
#include "D3D12DeviceContext.h"
#include "DescriptorHeap.h"
#include "Core\Performance\PerfManager.h"
#define ENABLE_CACHE 1

DescriptorCache::DescriptorCache(D3D12DeviceContext* con)
{
	Device = con;
	CacheHeap = new DescriptorHeap(con, 2048, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	PerfManager::Get()->AddTimer(TimerName, "RHI");
	PerfManager::Get()->AddTimer(ReuseTimer, "RHI");
}

void DescriptorCache::OnHeapClear()
{
	DescriptorsInHeap.clear();
}

DescriptorCache::~DescriptorCache()
{}


uint64 DescriptorCache::GetHash(const RSBind* bind)
{
	uint64 hash = 0;
	if (bind->BindType == ERSBindType::Texture)
	{
		std::string path = bind->Texture.Get()->TexturePath;
		HashUtils::hash_combine(hash, path);
	}
	HashUtils::hash_combine(hash, bind->Framebuffer);
	HashUtils::hash_combine(hash, bind->BufferTarget);
	HashUtils::hash_combine(hash, bind->TextureArray);
	if (bind->BindType == ERSBindType::TextureArray)
	{
		HashUtils::hash_combine(hash, D3D12RHI::DXConv(bind->TextureArray)->GetHash());
	}
	HashUtils::hash_combine(hash, bind->View.Mip);
	HashUtils::hash_combine(hash, bind->View.ArraySlice);
	HashUtils::hash_combine(hash, bind->View.MipLevels);
	HashUtils::hash_combine(hash, bind->View.Resource);
	HashUtils::hash_combine(hash, bind->View.Dimension);
	HashUtils::hash_combine(hash, bind->View.Offset);
	HashUtils::hash_combine(hash, bind->Offset);
	return hash;
}

bool DescriptorCache::FindInCache(uint64 hash, DXDescriptor** desc, ERSBindType::Type type)
{
	auto itor = TextureMap[type].find(hash);
	if (itor != TextureMap[type].end())
	{
		*desc = itor->second;
		return true;
	}
	return false;
}

DXDescriptor* DescriptorCache::CopyToCurrentHeap(DXDescriptor * d, bool CouldbeReused /*= true*/)
{
	auto itor = DescriptorsInHeap.find(d);
	if (itor != DescriptorsInHeap.end())
	{
		PerfManager::Get()->AddToCountTimer(ReuseTimer, 1);
		return itor->second;
	}
	PerfManager::Get()->AddToCountTimer(TimerName, 1);
	DXDescriptor* Copy = Device->GetHeapManager()->GetMainHeap()->CopyToHeap(d);
	DescriptorsInHeap.emplace(d, Copy);
	return Copy;
}

bool DescriptorCache::ShouldCache(const RSBind* bind)
{
	return true;
}
DXDescriptor* DescriptorCache::Create(const RSBind* bind, DescriptorHeap* heap)
{
	if (bind->BindType == ERSBindType::Texture)
	{
		return D3D12RHI::DXConv(bind->Texture.Get())->GetDescriptor(bind->View, heap);
	}
	else if (bind->BindType == ERSBindType::FrameBuffer)
	{
		return D3D12RHI::DXConv(bind->Framebuffer)->GetDescriptor(bind->View, heap);
	}
	else if (bind->BindType == ERSBindType::UAV)
	{
		if (bind->Framebuffer != nullptr)
		{
			return D3D12RHI::DXConv(bind->Framebuffer)->GetDescriptor(bind->View, heap);
		}
		else
		{
			return D3D12RHI::DXConv(bind->BufferTarget)->GetDescriptor(bind->View, heap);
		}
	}
	else if (bind->BindType == ERSBindType::TextureArray)
	{
		return D3D12RHI::DXConv(bind->TextureArray)->GetDescriptor(bind->View, heap);
	}
	else if (bind->BindType == ERSBindType::BufferSRV)
	{
		return D3D12RHI::DXConv(bind->BufferTarget)->GetDescriptor(bind->View, heap);
	}
	return nullptr;
}
DXDescriptor * DescriptorCache::GetOrCreate(const RSBind * bind)
{
	DXDescriptor* Desc = nullptr;
	if (bind->BindType == ERSBindType::RootConstant)
	{
		ensure(false);
	}
	if (!ShouldCache(bind))
	{
		return Create(bind, nullptr);
	}
	uint64 hash = GetHash(bind);
	if (FindInCache(hash, &Desc, bind->BindType))
	{
		return CopyToCurrentHeap(Desc);
	}
	Desc = Create(bind, CacheHeap);
	TextureMap[bind->BindType].emplace(hash, Desc);
	Desc = CopyToCurrentHeap(Desc, false);
	return Desc;
}
