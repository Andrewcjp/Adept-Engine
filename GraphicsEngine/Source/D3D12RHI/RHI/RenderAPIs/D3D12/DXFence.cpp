#include "DXFence.h"
#include "D3D12RHI.h"
#include "D3D12DeviceContext.h"

DXFence::DXFence(DeviceContext* con, EFenceFlags::Type flags)
{
	Context = D3D12RHI::DXConv(con);
	D3D12_FENCE_FLAGS Flags = D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE;
	if (flags == EFenceFlags::CrossAdaptor)
	{
		Flags = D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER | D3D12_FENCE_FLAG_SHARED;
	}
	Context->GetDevice()->CreateFence(0, Flags, ID_PASS(&m_Fence));
}

DXFence::~DXFence()
{}

void DXFence::Signal(EDeviceContextQueue::Type queue, uint64 value)
{
	Context->GetCommandQueueFromEnum(queue)->Signal(m_Fence, value);
	Value = value;
}

void DXFence::WaitForValue(EDeviceContextQueue::Type queue, uint64 value)
{
	Context->GetCommandQueueFromEnum(queue)->Wait(m_Fence, value);
}

void DXFence::Release()
{
	SafeRelease(m_Fence);
	RHIFence::Release();
}
