#include "stdafx.h"
#include "RHICommandList.h"
#include "Rendering/Core/FrameBuffer.h"
#include "DeviceContext.h"


RHICommandList::RHICommandList(ECommandListType::Type type)
{
	ListType = type;
}

RHICommandList::~RHICommandList()
{}

void RHICommandList::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{}

DeviceContext * RHICommandList::GetDevice()
{
	return Device;
}

int RHICommandList::GetDeviceIndex()
{
	if (Device != nullptr)
	{
		return Device->GetDeviceIndex();
	}
	return 0;
}
