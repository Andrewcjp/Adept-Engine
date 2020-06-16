#include "DescriptorCache.h"
#include "RHI\RHIRootSigniture.h"
#include "DXDescriptor.h"
#include "D3D12CommandList.h"
#include "DescriptorHeapManager.h"
#include "D3D12DeviceContext.h"
#include "DescriptorHeap.h"
#include "Core\Performance\PerfManager.h"
#include "Rendering\Core\FrameBuffer.h"
#include "D3D12Framebuffer.h"
#include "D3D12Buffer.h"
#include "Core\Utils\TypeUtils.h"
#include "D3D12RHITexture.h"
#include "D3D12Texture.h"

#define ENABLE_CACHE 1

DescriptorCache::DescriptorCache(D3D12DeviceContext* con)
{
	Device = con;
	CacheHeap = new DescriptorHeap(con, 512, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	PerfManager::Get()->AddTimer(TimerName, "RHI");
	PerfManager::Get()->AddTimer(ReuseTimer, "RHI");
	PerfManager::Get()->AddTimer(MissTimer, "RHI");
}

void DescriptorCache::OnHeapClear()
{
	DescriptorsInHeap.clear();
	RemoveInvalidCaches();
}

void DescriptorCache::Invalidate()
{
	for (int i = 0; i < ERSBindType::Limit; i++)
	{
		DescriptorMap[i].clear();
	}
	DescriptorsInHeap.clear();
	CacheHeap->ClearHeap();
}

void DescriptorCache::RemoveInvalidCaches()
{
	//this is removing items wrong
	//possible - Heap is overwriting
	//todo optimise;
	return;
	for (int i = 0; i < ERSBindType::Limit; i++)
	{
		for (auto itor = DescriptorMap[i].begin(); itor != DescriptorMap[i].end();)
		{
			if (itor->second.LastUsedFrame + 50 < RHI::GetFrameCount())
			{
				CacheHeap->RemoveDescriptor(itor->second.desc);
				DescriptorMap[i].erase(itor++);    // or "it = m.erase(it)" since C++11
			}
			else
			{
				++itor;
			}
		}
	}
}

DescriptorCache::~DescriptorCache()
{}

uint64 DescriptorCache::GetHash(const RSBind* bind)
{
	uint64 hash = 0;
	if (bind->BindType == ERSBindType::Texture2)
	{
		HashUtils::hash_combine(hash, DXDescriptor::GetItemDescHash(D3D12RHI::DXConv(bind->Texture2)->GetItemDesc(bind->View)));
	}
	else if (bind->BindType == ERSBindType::Texture)
	{
		HashUtils::hash_combine(hash, DXDescriptor::GetItemDescHash(D3D12RHI::DXConv(bind->Texture.Get())->GetItemDesc(bind->View)));
	}
	else if (bind->BindType == ERSBindType::FrameBuffer)
	{
		HashUtils::hash_combine(hash, D3D12RHI::DXConv(bind->Framebuffer)->GetViewHash(bind->View));
	}
	else if (bind->BindType == ERSBindType::TextureArray)
	{
		HashUtils::hash_combine(hash, D3D12RHI::DXConv(bind->TextureArray)->GetHash());
	}
	else if (bind->BindType == ERSBindType::UAV)
	{
		if (bind->Texture2)
		{
			HashUtils::hash_combine(hash, DXDescriptor::GetItemDescHash(D3D12RHI::DXConv(bind->Texture2)->GetItemDesc(bind->View)));
		}
	}
	return hash;
}

bool DescriptorCache::FindInCache(uint64 hash, DXDescriptor** desc, ERSBindType::Type type)
{
	auto itor = DescriptorMap[type].find(hash);
	if (itor != DescriptorMap[type].end())
	{
		*desc = itor->second.desc;
		itor->second.LastUsedFrame = RHI::GetFrameCount();
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
	return false;
	//todo: fix issue with caching framebuffer descriptors
	//GPU hang :(
	return true;// bind->BindType != ERSBindType::FrameBuffer;
}

DXDescriptor* DescriptorCache::Create(const RSBind* bind, DescriptorHeap* heap)
{
	if (bind->BindType != ERSBindType::TextureArray)
	{
		ensure(bind->View.ViewType != EViewType::Limit);
	}
	PerfManager::Get()->AddToCountTimer(MissTimer, 1);
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
		else if (bind->BufferTarget != nullptr)
		{
			return D3D12RHI::DXConv(bind->BufferTarget)->GetDescriptor(bind->View, heap);
		}
		else if (bind->Texture2)
		{
			return D3D12RHI::DXConv(bind->Texture2)->GetDescriptor(bind->View, heap);
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
	else if (bind->BindType == ERSBindType::Texture2)
	{
		return D3D12RHI::DXConv(bind->Texture2)->GetDescriptor(bind->View, heap);
	}
	return nullptr;
}
DXDescriptor* DescriptorCache::FindInCacheHeap(const RSBind* bind, bool ForceCache)
{
	DXDescriptor* Desc = nullptr;
	if (!ShouldCache(bind) && !ForceCache)
	{
		return Create(bind, CacheHeap);
	}
	uint64 hash = GetHash(bind);
	if (FindInCache(hash, &Desc, bind->BindType))
	{
		return Desc;
	}
	Desc = Create(bind, CacheHeap);
	DescriptorRef ref;
	ref.LastUsedFrame = RHI::GetFrameCount();
	ref.desc = Desc;
	DescriptorMap[bind->BindType].emplace(hash, ref);
	ensure(Desc->IsValid());
	return Desc;
}

DXDescriptor* DescriptorCache::GetOrCreateNull(const RSBind* bind)
{
	DescriptorItemDesc Desc;
	D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
	destTextureUAVDesc.ViewDimension = D3D12Helpers::ConvertDimensionUAV(bind->View.Dimension);
	
	destTextureUAVDesc.Format = D3D12Helpers::ConvertFormat(bind->View.Format);
	if (destTextureUAVDesc.ViewDimension == D3D12_UAV_DIMENSION_UNKNOWN)
	{
		destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	}
	destTextureUAVDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
	if (!bind->View.UseResourceFormat)
	{
		destTextureUAVDesc.Format = D3D12Helpers::ConvertFormat(bind->View.Format);
	}
	destTextureUAVDesc.Texture2D.MipSlice = bind->View.Mip;
	Desc.CreateUnorderedAccessView(nullptr, nullptr, &destTextureUAVDesc);
	DXDescriptor* Descriptor = Device->GetHeapManager()->GetMainHeap()->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
	Descriptor->SetItemDesc(Desc);
	Descriptor->Recreate();
	return Descriptor;
}

DXDescriptor* DescriptorCache::GetOrCreate(const RSBind* bind)
{
	std::lock_guard<std::mutex> lock(CacheLock);
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
	DescriptorRef ref;
	ref.LastUsedFrame = RHI::GetFrameCount();
	ref.desc = Desc;
	DescriptorMap[bind->BindType].emplace(hash, ref);
	Desc = CopyToCurrentHeap(Desc, false);
	ensure(Desc->IsValid());
	return Desc;
}
