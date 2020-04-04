#include "Stdafx.h"
#include "RHIFence.h"

RHIFence::RHIFence()
{
	Value++;
}

RHIFence::~RHIFence()
{
}

void RHIFence::Signal(DeviceContextQueue::Type queue, uint64 value)
{
	Value = value;
}

void RHIFence::WaitForValue(DeviceContextQueue::Type queue, uint64 value)
{
}

uint64 RHIFence::GetValue() const
{
	return Value;
}

void RHIFence::SetValue(uint64 value)
{
	Value = value;
}
