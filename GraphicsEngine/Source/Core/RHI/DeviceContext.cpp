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

void DeviceContext::DestoryDevice()
{

}

const CapabilityData & DeviceContext::GetCaps()
{
	return Caps_Data;
}

RHICommandList * DeviceContext::GetInterGPUCopyList()
{
	return InterGPUCopyList;
}
