
#include "RHICommandList.h"
#include "DeviceContext.h"
#include "Core/Platform/PlatformCore.h"
#include "RHITypes.h"
#include "Shader.h"
#include "Core/Utils/StringUtil.h"

RHICommandList::RHICommandList(ECommandListType::Type type, DeviceContext* context)
{
	ListType = type;
#if NAME_RHI_PRIMS
	std::string CopyListletter = "";
	switch (ListType)
	{
		case ECommandListType::Graphics:
			CopyListletter = "G";
			break;
		case ECommandListType::Compute:
			CopyListletter = "C";
			break;
		case ECommandListType::Copy:
			CopyListletter = "CPY";
			break;
		case ECommandListType::RayTracing:
			CopyListletter = "RT";
			break;
	}
	std::string data = "(CMDLIST-" + CopyListletter + " DEV:" + std::to_string(context->GetDeviceIndex()) + ")";
	ObjectSufix = StringUtils::CopyStringToCharArray(data);
#endif
	Device = context;
}

RHICommandList::~RHICommandList()
{}

void RHICommandList::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{

}

void RHICommandList::DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation)
{

}

void RHICommandList::BeginRenderPass(RHIRenderPassDesc& RenderPass)
{
	ensure(!IsInRenderPass);
	IsInRenderPass = true;
}

void RHICommandList::EndRenderPass()
{
	ensure(IsInRenderPass);
	IsInRenderPass = false;
}

DeviceContext * RHICommandList::GetDevice()
{
	return Device;
}

int RHICommandList::GetDeviceIndex() const
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
		GetDevice()->GetTimeManager()->StartTotalGPUTimer(this);
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
	/*if (ListType == ECommandListType::Copy)
	{
		if (GetDevice()->GetCaps().SupportsCopyTimeStamps)
		{
			GetDevice()->GetTimeManager()->ResolveCopyTimeHeaps(this);
		}
	}
	else
	{
		GetDevice()->GetTimeManager()->ResolveTimeHeaps(this);
	}*/
}

bool RHICommandList::IsGraphicsList() const
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
bool RHICommandList::IsRaytracingList() const
{
	return ListType == ECommandListType::RayTracing;
}

void RHICommandList::InsertGPUStallTimer()
{
	if (RHI::GetDeviceCount() > 1)
	{
		StartTimer(EGPUTIMERS::GPU0WaitOnGPU1);
		Device->InsertStallTimerMarker();
	}
}

void RHICommandList::HandleStallTimer()
{
	if (Device->ShouldInsertTimer() && !IsCopyList() && RHI::GetDeviceCount() > 1)
	{
		EndTimer(EGPUTIMERS::GPU0WaitOnGPU1);
		Device->OnInsertStallTimer();
	}
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

const RHIViewDesc& RHIUAV::GetViewDesc()const
{
	return ViewDesc;
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



RHIRenderPass::RHIRenderPass(RHIRenderPassDesc & desc)
{
	Desc = desc;
}

RHIRenderPass::~RHIRenderPass()
{}

void RHIRenderPass::AddSubPass(RHISubPass * Pass)
{
	SubPasses.push_back(Pass);
}

void RHIRenderPass::Complie()
{}


void RHICommandList::SetRHIBufferReadOnly(RHIBuffer * buffer, int slot)
{
	buffer->BindBufferReadOnly(this, slot);
}

void RHICommandList::SetUAV(RHIUAV * uav, int slot)
{
	uav->Bind(this, slot);
}

void RHICommandList::SetConstantBufferView(RHIBuffer * buffer, int offset, std::string Slot)
{
	ensure(CurrentPSO);
	int index = CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(Slot);
	if (index == -1)
	{
		return;
	}
	SetConstantBufferView(buffer, offset, index);
}

void RHICommandList::SetTexture(BaseTextureRef texture, std::string slot)
{
	ensure(CurrentPSO);
	SetTexture(texture, CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(slot));
}

void RHICommandList::SetFrameBufferTexture(FrameBuffer * buffer, std::string slot, int Resourceindex)
{
	ensure(CurrentPSO);
	int index = CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(slot);
	if (index == -1)
	{
		return;
	}
	SetFrameBufferTexture(buffer, index, Resourceindex);
}

void RHICommandList::SetRHIBufferReadOnly(RHIBuffer * buffer, std::string slot)
{
	ensure(CurrentPSO);
	int index = CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(slot);
	if (index == -1)
	{
		return;
	}
	SetRHIBufferReadOnly(buffer, index);
}

void RHICommandList::SetUAV(RHIUAV * uav, std::string slot)
{
	ensure(CurrentPSO);
	ensure(uav);
	SetUAV(uav, CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(slot));
}
