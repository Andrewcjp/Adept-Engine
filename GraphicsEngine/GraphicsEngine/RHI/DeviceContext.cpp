#include "Stdafx.h"
#include "DeviceContext.h"
#include "Core/Asserts.h"
#include "RHITypes.h"
#include "RHITimeManager.h"

DeviceContext::DeviceContext()
{}

DeviceContext::~DeviceContext()
{}


void DeviceContext::ResetDeviceAtEndOfFrame()
{
	if (CurrentFrameIndex == 0)
	{
		GetTimeManager()->UpdateTimers();
	}
}

void DeviceContext::MoveNextFrame()
{
	CurrentFrameIndex++;
	if (CurrentFrameIndex == RHI::CPUFrameCount - 1)
	{
		CurrentFrameIndex = 0;
	}
}

void DeviceContext::DestoryDevice()
{

}

RHICommandList * DeviceContext::GetInterGPUCopyList()
{
	return GPUCopyList;
}
