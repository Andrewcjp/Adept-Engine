#pragma once
#include "RHI/RHIFence.h"

class D3D12DeviceContext;
class DXFence : public RHIFence
{
public:
	DXFence(DeviceContext* con, EFenceFlags::Type Flags);
	~DXFence();
	void Signal(EDeviceContextQueue::Type queue, uint64 value)override;
	void WaitForValue(EDeviceContextQueue::Type queue, uint64 value) override;


	void Release() override;

private:
	ID3D12Fence* m_Fence = nullptr;
	D3D12DeviceContext* Context = nullptr;
};

