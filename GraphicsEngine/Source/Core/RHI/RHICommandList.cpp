#include "stdafx.h"
#include "RHICommandList.h"
#include "Rendering/Core/FrameBuffer.h"
#include "DeviceContext.h"
#include "Core/Platform/PlatformCore.h"

RHICommandList::RHICommandList(ECommandListType::Type type, DeviceContext* context)
{
	ListType = type;
	ObjectSufix = "(CMDLIST)";
	Device = context;
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

void RHICommandList::StartTimer(int TimerId)
{
	if (ListType == ECommandListType::Copy)
	{
		if (GetDevice()->GetCaps().SupportsCopyTimeStamps)
		{
			GetDevice()->GetTimeManager()->StartTimer(this, TimerId);
		}
	}
	else
	{
		GetDevice()->GetTimeManager()->StartTimer(this, TimerId);
	}
}

void RHICommandList::EndTimer(int TimerId)
{
	if (ListType == ECommandListType::Copy)
	{
		if (GetDevice()->GetCaps().SupportsCopyTimeStamps)
		{
			GetDevice()->GetTimeManager()->EndTimer(this, TimerId);
		}
	}
	else
	{
		GetDevice()->GetTimeManager()->EndTimer(this, TimerId);
	}
}

void RHICommandList::ResolveTimers()
{
	if (ListType == ECommandListType::Copy)
	{
		if (GetDevice()->GetCaps().SupportsCopyTimeStamps)
		{
			GetDevice()->GetTimeManager()->ResolveCopyTimeHeaps(this);
		}
	}
	else
	{

	}
}

bool RHICommandList::IsGraphicsList()const
{
	return ListType == ECommandListType::Graphics;
}

bool RHICommandList::IsCopyList() const
{
	return ListType == ECommandListType::Copy;
}

bool RHICommandList::IsComputeList() const
{
	return ListType == ECommandListType::Compute;
}

RHIUAV * RHIBuffer::GetUAV()
{
	ensureMsgf(Desc.CreateUAV, "CreateUAV should be set on this buffer to use a UAV from it");
	return UAV;
}

RHIUAV::RHIUAV()
{
	ObjectSufix = "(UAV)";
}

RHIBuffer::RHIBuffer(ERHIBufferType::Type type)
{
	CurrentBufferType = type;	
	switch (CurrentBufferType)
	{
	case ERHIBufferType::Constant:
		ObjectSufix = "(Constant Buffer)";
		break;
	case ERHIBufferType::Vertex:
		ObjectSufix = "(VTX Buffer)";
		break;
	case ERHIBufferType::Index:
		ObjectSufix = "(IDX Buffer)";
		break;
	case ERHIBufferType::GPU:
		ObjectSufix = "(GPU Buffer)";
	default:
		break;
	}
}

RHITextureArray::RHITextureArray(DeviceContext * device, int inNumEntries)
{
	NumEntries = inNumEntries;
	ObjectSufix = "(TexArray)";
}
