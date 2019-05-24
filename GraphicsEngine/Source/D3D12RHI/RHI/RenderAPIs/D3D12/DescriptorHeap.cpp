
#include "DescriptorHeap.h"
#include "D3D12DeviceContext.h"
#include "D3D12RHI.h"
#include "D3D12CommandList.h"
#include "Descriptor.h"
CreateChecker(DescriptorHeap);
DescriptorHeap::DescriptorHeap(DescriptorHeap * other, int newsize) :
	DescriptorHeap(other->Device, newsize, other->srvHeapDesc.Type, other->srvHeapDesc.Flags)
{

}


DescriptorHeap::DescriptorHeap(DeviceContext* inDevice, int Num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	ensure(Num > 0);
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
	if (!IsReleased)
	{
		Release();
	}
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

void DescriptorHeap::BindHeap_Old(ID3D12GraphicsCommandList * list)
{
	//return;
	ID3D12DescriptorHeap* ppHeaps[] = { mHeap };
	list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}

void DescriptorHeap::AddDescriptor(Descriptor * desc)
{
	//validate type.
	desc->indexInHeap = GetNextFreeIndex();
	desc->SetOwner(this);
	desc->Recreate();
	ContainedDescriptors.push_back(desc);
}

int DescriptorHeap::GetNumberOfDescriptors()
{
	return (int)ContainedDescriptors.size();
}

int DescriptorHeap::GetMaxSize()
{
	return srvHeapDesc.NumDescriptors;
}

int DescriptorHeap::GetNextFreeIndex()
{
	int Count = 0;
	for (int i = 0; i < ContainedDescriptors.size(); i++)
	{
		Count += ContainedDescriptors[i]->GetSize();
	}
	return Count;
}

void DescriptorHeap::MoveAllToHeap(DescriptorHeap * heap, int offset)
{
	for (int i = 0; i < ContainedDescriptors.size(); i++)
	{
		heap->AddDescriptor(ContainedDescriptors[i]);
	}
	bool IsCPUWriteOnly = srvHeapDesc.Flags |= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (!IsCPUWriteOnly)
	{
		Device->GetDevice()->CopyDescriptorsSimple(heap->GetNextFreeIndex(), heap->mHeap->GetCPUDescriptorHandleForHeapStart(), mHeap->GetCPUDescriptorHandleForHeapStart(), srvHeapDesc.Type);
	}
	else
	{
		//recreate in new heap
		for (int i = 0; i < heap->ContainedDescriptors.size(); i++)
		{
			heap->ContainedDescriptors[i]->Recreate();
		}
	}
}

D3D12DeviceContext * DescriptorHeap::GetDevice()
{
	return Device;
}

void DescriptorHeap::RemoveDescriptor(Descriptor * desc)
{
	VectorUtils::Remove(ContainedDescriptors, desc);
	//#Descriptors: handle defragmentation of descriptor heaps.
}

void DescriptorHeap::BindHeap(D3D12CommandList * list)
{
	list->AddHeap(this);
	list->PushHeaps();
}
void DescriptorHeap::Release()
{
	IRHIResourse::Release();
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

ID3D12DescriptorHeap * DescriptorHeap::GetHeap()
{
	return mHeap;
}

std::string DescriptorHeap::GetDebugName()
{
	return "DescriptorHeap";
}
