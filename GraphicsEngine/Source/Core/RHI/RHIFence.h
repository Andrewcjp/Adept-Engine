#pragma once
#include "RHITypes.h"
class RHIFence :public IRHIResourse
{
public:
	RHI_API RHIFence();
	RHI_API virtual ~RHIFence();
	virtual void Signal(EDeviceContextQueue::Type queue, uint64 value);
	virtual void WaitForValue(EDeviceContextQueue::Type queue, uint64 value);
	RHI_API uint64 GetValue() const;
	RHI_API void SetValue(uint64 value);
protected:
	uint64 Value = 0;
};
namespace EFenceFlags
{
	enum Type
	{
		None,
		CrossAdaptor
	};
}

