
#include "DescriptorHeap.h"
#include <algorithm>
#include "D3D12DeviceContext.h"
#include "D3D12RHI.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Utils/RefChecker.h"
CreateChecker(DescriptorHeap);
DescriptorHeap::DescriptorHeap(DeviceContext* inDevice, int Num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	Device = (D3D12DeviceContext*)inDevice;
	srvHeapDesc.NumDescriptors = std::max(Num, 1);
	DescriptorCount = Num;
	srvHeapDesc.Type = type;
	srvHeapDesc.Flags = flags;
	ThrowIfFailed(Device->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mHeap)));
	DescriptorOffsetSize = Device->GetDevice()->GetDescriptorHandleIncrementSize(type);
	const std::string name = "Desc Heap Descs: " + std::to_string(srvHeapDesc.NumDescriptors);
	SetName(StringUtils::ConvertStringToWide(name).c_str());
	AddCheckerRef(DescriptorHeap, this);
}

DescriptorHeap::~DescriptorHeap()
{
	Release();
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGpuAddress(int index)
{
	ensure(index < DescriptorCount);
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeap->GetGPUDescriptorHandleForHeapStart(), index, DescriptorOffsetSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUAddress(int index)
{
	ensure(index < DescriptorCount);
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeap->GetCPUDescriptorHandleForHeapStart(), index, DescriptorOffsetSize);
}

void DescriptorHeap::SetName(LPCWSTR name)
{
	mHeap->SetName(name);
}

void DescriptorHeap::BindHeap(ID3D12GraphicsCommandList * list)
{
	ID3D12DescriptorHeap* ppHeaps[] = { mHeap };
	list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void DescriptorHeap::Release()
{
	if (mHeap)
	{
		RemoveCheckerRef(DescriptorHeap, this);
		SafeRelease(mHeap);
	}
}

void DescriptorHeap::SetPriority(EGPUMemoryPriority NewPriority)
{
	Priority = NewPriority;
}
