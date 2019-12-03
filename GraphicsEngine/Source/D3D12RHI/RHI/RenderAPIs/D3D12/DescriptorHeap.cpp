
#include "DescriptorHeap.h"
#include "D3D12DeviceContext.h"
#include "D3D12RHI.h"
#include "D3D12CommandList.h"
#include "DXDescriptor.h"
CreateChecker(DescriptorHeap);
DescriptorHeap::DescriptorHeap(DescriptorHeap * other, int newsize) :
	DescriptorHeap(other->Device, newsize, other->HeapDesc.Type, other->HeapDesc.Flags)
{

}


DescriptorHeap::DescriptorHeap(DeviceContext* inDevice, int Num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	ensure(Num > 0);
	Device = D3D12RHI::DXConv(inDevice);
	HeapDesc.NumDescriptors = std::max(Num, 1);
	DescriptorCount = Num;
	HeapDesc.Type = type;
	HeapDesc.Flags = flags;
	CreateHeap();

	AddCheckerRef(DescriptorHeap, this);
}

void DescriptorHeap::CreateHeap()
{
	ThrowIfFailed(Device->GetDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&mHeap)));
	DescriptorOffsetSize = Device->GetDevice()->GetDescriptorHandleIncrementSize(HeapDesc.Type);
	const std::string name = "Desc Heap Descs: " + std::to_string(HeapDesc.NumDescriptors);
	SetName(StringUtils::ConvertStringToWide(name).c_str());
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

void DescriptorHeap::AddDescriptor(DXDescriptor* desc, bool Create /*= true*/)
{
	//validate type.
	desc->indexInHeap = GetNextFreeIndex();
	desc->SetOwner(this);
	if (Create)
	{
		desc->Recreate();
	}
	ContainedDescriptors.push_back(desc);
}

int DescriptorHeap::GetNumberOfDescriptors()
{
	return (int)ContainedDescriptors.size();
}

int DescriptorHeap::GetMaxSize()
{
	return HeapDesc.NumDescriptors;
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
	bool IsCPUWriteOnly = HeapDesc.Flags |= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (!IsCPUWriteOnly)
	{
		Device->GetDevice()->CopyDescriptorsSimple(heap->GetNextFreeIndex(), heap->mHeap->GetCPUDescriptorHandleForHeapStart(), mHeap->GetCPUDescriptorHandleForHeapStart(), HeapDesc.Type);
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

void DescriptorHeap::RemoveDescriptor(DXDescriptor * desc)
{
	VectorUtils::Remove(ContainedDescriptors, desc);
	desc->PendingRemoval = true;
	//#Descriptors: handle defragmentation of descriptor heaps.
}

void DescriptorHeap::ClearHeap()
{
	MemoryUtils::DeleteVector(ContainedDescriptors);
	ContainedDescriptors.clear();
}

DXDescriptor* DescriptorHeap::CopyToHeap(DXDescriptor * desc)
{
	DXDescriptor* Copy = AllocateDescriptor(desc->GetType(), desc->GetSize());
	Copy->InitFromDesc(desc);
	Device->GetDevice()->CopyDescriptorsSimple(desc->GetSize(), Copy->GetCPUAddress(), desc->GetCPUAddress(), HeapDesc.Type);
	return Copy;
}

DXDescriptor* DescriptorHeap::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, int size)
{
	if (GetNextFreeIndex() + size >= GetMaxSize())
	{
		ensure(false);
	}
	DXDescriptor* D = new DXDescriptor();
	D->Init(type, this, size);
	AddDescriptor(D, false);
	return D;
}

void DescriptorHeap::BindHeap(D3D12CommandList * list)
{
	//#todo: sampler heaps
	list->ClearHeaps();
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
