#include "D3D12RHIPCH.h"
#include "DescriptorGroup.h"
#include "DescriptorHeap.h"
#include "D3D12DeviceContext.h"
#include "Descriptor.h"


DescriptorGroup::DescriptorGroup()
{
	Descriptors[0] = new Descriptor();
	Descriptors[1] = new Descriptor();
}


DescriptorGroup::~DescriptorGroup()
{}

void DescriptorGroup::Init(D3D12_DESCRIPTOR_HEAP_TYPE Type, DescriptorHeap * heap, int size)
{
	context = heap->GetDevice();
	Descriptors[0]->Init(Type, heap, size);
	Descriptors[1]->Init(Type, heap, size);
	CreatedThisFrame = true;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorGroup::GetGPUAddress(int index)
{
	return Descriptors[context->GetCpuFrameIndex()]->GetGPUAddress(index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorGroup::GetCPUAddress(int index)
{
	return  Descriptors[context->GetCpuFrameIndex()]->GetCPUAddress(index);
}

D3D12_DESCRIPTOR_HEAP_TYPE DescriptorGroup::GetType()
{
	return  Descriptors[context->GetCpuFrameIndex()]->GetType();
}

int DescriptorGroup::GetSize()
{
	return Descriptors[context->GetCpuFrameIndex()]->GetSize();
}

void DescriptorGroup::Recreate()
{
	Descriptors[context->GetCpuFrameIndex()]->Recreate();
	RecreateQueued = true;
}

void DescriptorGroup::CreateShaderResourceView(ID3D12Resource * pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC * pDesc, int offset)
{
	Descriptors[0]->CreateShaderResourceView(pResource, pDesc, offset);
	Descriptors[1]->CreateShaderResourceView(pResource, pDesc, offset);

	if (CreatedThisFrame)
	{
		Descriptors[0]->Recreate();
		Descriptors[1]->Recreate();
		CreatedThisFrame = false;
	}
	else
	{
		Descriptors[context->GetCpuFrameIndex()]->Recreate();
	}
	RecreateQueued = true;
}

void DescriptorGroup::CreateUnorderedAccessView(ID3D12Resource * pResource, ID3D12Resource * pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC * pDesc, int offset)
{
	Descriptors[0]->CreateUnorderedAccessView(pResource, pCounterResource, pDesc, offset);
	Descriptors[1]->CreateUnorderedAccessView(pResource, pCounterResource, pDesc, offset);
	if (CreatedThisFrame)
	{
		Descriptors[0]->Recreate();
		Descriptors[1]->Recreate();
		CreatedThisFrame = false;
	}
	else
	{
		Descriptors[context->GetCpuFrameIndex()]->Recreate();
	}
	RecreateQueued = true;
}

//called when the frame switches to ensure the B copy is updated with changes made last CPU frame.
void DescriptorGroup::OnFrameSwitch()
{
	//ensure(Descriptors[lastFrame]->NeedsUpdate() == false);
	if (RecreateQueued || Descriptors[context->GetCpuFrameIndex()]->NeedsUpdate())
	{
		Descriptors[context->GetCpuFrameIndex()]->Recreate();
		RecreateQueued = false;
	}
	lastFrame = RHI::GetFrameCount();
}

Descriptor * DescriptorGroup::GetDescriptor(int index)
{
	return Descriptors[index];
}

void DescriptorGroup::Release()
{
	Descriptors[0]->Release();
	Descriptors[1]->Release();
}
