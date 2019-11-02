
#include "RHICommandList.h"
#include "DeviceContext.h"
#include "Core/Platform/PlatformCore.h"
#include "Shader.h"
#include "Core/Utils/StringUtil.h"
#include "Rendering/Core/VRXEngine.h"
#include "Rendering/Core/FrameBuffer.h"
#include "RHITimeManager.h"

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

void RHICommandList::BeginRenderPass(const RHIRenderPassDesc& RenderPass)
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
		GetDevice()->GetTimeManager()->StartTotalGPUTimer(this);
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

void RHICommandList::SetHighLevelAccelerationStructure(HighLevelAccelerationStructure * Struct)
{

}

void RHICommandList::TraceRays(const RHIRayDispatchDesc& desc)
{

}

void RHICommandList::SetStateObject(RHIStateObject* Object)
{

}

void RHICommandList::ResolveVRXFramebuffer(FrameBuffer * Target)
{
	//ensure(Target->GetDescription().VarRateSettings.BufferMode != FrameBufferVariableRateSettings::None);
	//#VRX: Setting for VRS mode
	//Native VRS does not require resolve
	SetVRSShadingRate(VRS_SHADING_RATE::SHADING_RATE_4X2);
	if (Target->GetDescription().VarRateSettings.BufferMode == FrameBufferVariableRateSettings::VRR)
	{
		VRXEngine::ResolveVRRFramebuffer(this, Target);
	}
	else
	{
		if (Device->GetCaps().VRSSupport == EVRSSupportType::None || !RHI::GetRenderSettings()->AllowNativeVRS)
		{
			VRXEngine::Get()->ResolveVRSFramebuffer(this, Target);
		}
	}
}

void RHICommandList::SetVRSShadingRate(VRS_SHADING_RATE::type Rate)
{
	if (Device->GetCaps().VRSSupport == EVRSSupportType::Hardware && RHI::GetRenderSettings()->AllowNativeVRS)
	{
		SetVRSShadingRateNative(Rate);
	}
	else
	{
		VRXEngine::Get()->SetVRSShadingRate(this, Rate);
	}
}

void RHICommandList::SetVRRShadingRate(int RateIndex)
{
	VRXEngine::Get()->SetVRRShadingRate(this, RateIndex);
}

void RHICommandList::SetVRXShadingRateImage(FrameBuffer * Target)
{
	if (Device->GetCaps().VRSSupport == EVRSSupportType::Hardware && Target->GetDescription().VarRateSettings.BufferMode == FrameBufferVariableRateSettings::VRS && RHI::GetRenderSettings()->AllowNativeVRS)
	{
		SetVRSShadingRateImageNative(Target);
	}
	else
	{
		VRXEngine::Get()->SetVRXShadingRateImage(this, Target);
	}
}

RHIPipeLineStateObject * RHICommandList::GetCurrnetPSO()
{
	return CurrentPSO;
}

ECommandListType::Type RHICommandList::GetListType() const
{
	return ListType;
}

void RHICommandList::SetVRSShadingRateNative(VRS_SHADING_RATE::type Rate)
{
	NOAPIIMP(SetVRSShadingRateNative);
}

void RHICommandList::SetVRSShadingRateImageNative(FrameBuffer * Target)
{
	NOAPIIMP(SetVRSShadingRateImageNative);
}

void RHIBuffer::Release()
{}



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



RHIRenderPass::RHIRenderPass(const RHIRenderPassDesc & desc)
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
	SetBuffer(buffer, slot, RHIViewDesc::DefaultSRV());
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

void RHICommandList::SetTexture(BaseTextureRef texture, std::string slot, int mip)
{
	ensure(CurrentPSO);
	SetTexture(texture, CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(slot), mip);
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

void RHICommandList::SetUAV(RHIBuffer * uav, std::string slot)
{
	SetUAV(uav, CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(slot), RHIViewDesc::DefaultUAV());
}

void RHICommandList::SetUAV(RHIBuffer * uav, int slot)
{
	SetUAV(uav, slot, RHIViewDesc::DefaultUAV());
}

void RHICommandList::SetUAV(FrameBuffer * uav, std::string slot, int ResourceIndex, int Face, int MipSlice)
{
	SetUAV(uav, CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(slot), ResourceIndex, Face, MipSlice);
}

void RHICommandList::SetBuffer(RHIBuffer * Buffer, int slot, int ElementOffset)
{
	RHIViewDesc view = RHIViewDesc::DefaultSRV();
	view.Offset = ElementOffset;
	SetBuffer(Buffer, slot, view);
}

void RHICommandList::SetBuffer(RHIBuffer * Buffer, std::string slot, int ElementOffset)
{
	SetBuffer(Buffer, CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(slot), ElementOffset);
}

void RHICommandList::SetTexture(BaseTextureRef texture, int slot, int mip)
{
	RHIViewDesc v = RHIViewDesc::DefaultSRV();
	v.Mip = mip;
	SetTexture(texture, slot, v);
}

void RHICommandList::SetUAV(FrameBuffer * uav, int slot, int ResourceIndex, int Face, int MipSlice)
{
	RHIViewDesc view;
	view.ArraySlice = Face;
	view.Mip = MipSlice;
	view.Resource = ResourceIndex;
	SetUAV(uav, slot, view);
}

void RHICommandList::SetCommandSigniture(RHICommandSignitureDescription desc)
{}
