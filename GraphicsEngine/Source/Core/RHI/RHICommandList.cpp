
#include "RHICommandList.h"
#include "DeviceContext.h"
#include "Core/Platform/PlatformCore.h"
#include "Shader.h"
#include "Core/Utils/StringUtil.h"
#include "Rendering/Core/VRXEngine.h"
#include "Rendering/Core/FrameBuffer.h"
#include "RHITimeManager.h"
#include "RHITexture.h"

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

void RHICommandList::DrawIndexedPrimitive(uint IndexCountPerInstance, uint InstanceCount, uint StartIndexLocation, uint BaseVertexLocation, uint StartInstanceLocation)
{

}

void RHICommandList::SetRootConstant(std::string SignitureSlot, int ValueNum, void * Data, int DataOffset)
{
	ensure(CurrentPSO);
	SetRootConstant(CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(SignitureSlot), ValueNum, Data, DataOffset);
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

bool RHICommandList::IsOpen() const
{
	return false;
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
	if (Target->GetDescription().VarRateSettings.BufferMode == FrameBufferVariableRateSettings::VRR)
	{
		VRXEngine::ResolveVRRFramebuffer(this, Target, GetShadingRateImage());
	}
}

void RHICommandList::SetVRSShadingRate(VRX_SHADING_RATE::type Rate)
{
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRS(GetDevice()))
	{
		SetVRSShadingRateNative(Rate);
	}
}

void RHICommandList::SetVRRShadingRate(int RateIndex)
{
	VRXEngine::Get()->SetVRRShadingRate(this, RateIndex);
}

void RHICommandList::PrepareFramebufferForVRR(RHITexture * RateImage, FrameBuffer* VRRTarget)
{
	ShadingRateImage = RateImage;
	if (!IsGraphicsList())
	{
		return;
	}
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRS(Device))
	{
		//VRS overrides VRR
		return;
	}
	if (VRRTarget != nullptr)
	{
		VRXEngine::WriteVRRStencil(this, VRRTarget);
	}
}

void RHICommandList::SetVRXShadingRateImage(RHITexture * RateImage)
{
	ShadingRateImage = RateImage;
	if (!IsGraphicsList())
	{
		return;
	}
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRS(Device))
	{
		SetVRSShadingRateImageNative(RateImage);
	}
	else if(RHI::GetRenderSettings()->GetVRXSettings().UseVRR(Device))
	{
		VRXEngine::Get()->SetVRXShadingRateImage(this, RateImage);
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

void RHICommandList::FlushBarriers()
{}

void RHICommandList::SetVRSShadingRateNative(VRX_SHADING_RATE::type Rate)
{
	NOAPIIMP(SetVRSShadingRateNative);
}

void RHICommandList::SetVRSShadingRateImageNative(RHITexture * Target)
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

void RHICommandList::SetTexture2(RHITexture * buffer, std::string slot)
{
	ensure(CurrentPSO);
	int index = CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(slot);
	if (index == -1)
	{
		return;
	}
	SetTexture2(buffer, index, RHIViewDesc::DefaultSRV());
}

void RHICommandList::SetTexture2(RHITexture * buffer, int slot)
{
	SetTexture2(buffer, slot, RHIViewDesc::DefaultSRV(DIMENSION_TEXTURE3D));
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

void RHICommandList::SetUAV(RHITexture * uav, std::string slot, int ResourceIndex, int Face, int MipSlice)
{
	RHIViewDesc desc = RHIViewDesc::DefaultUAV();
	desc.ArraySlice = Face;
	desc.Mip = MipSlice;
	desc.Dimension = uav->GetDescription().Dimension;
	SetUAV(uav, CurrentPSO->GetDesc().ShaderInUse->GetSlotForName(slot), desc);
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

void RHICommandList::SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex)
{
	RHIViewDesc v = RHIViewDesc::DefaultSRV();
	v.ResourceIndex = Resourceindex;
	SetFrameBufferTexture(buffer, slot, v);
}

void RHICommandList::SetUAV(FrameBuffer * uav, int slot, int ResourceIndex, int Face, int MipSlice)
{
	RHIViewDesc view;
	view.Dimension = DIMENSION_TEXTURE2D;
	view.ArraySlice = Face;
	view.Mip = MipSlice;
	view.ResourceIndex = ResourceIndex;
	SetUAV(uav, slot, view);
}

void RHICommandList::DispatchSized(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ)
{
	if (CurrentPSO != nullptr && CurrentPSO->GetDesc().ShaderInUse != nullptr)
	{
		glm::ivec3 DispatchGroupSize = CurrentPSO->GetDesc().ShaderInUse->GetShaderProgram()->GetComputeThreadSize();
		Dispatch(glm::ceil((float)ThreadGroupCountX / (float)DispatchGroupSize.x), glm::ceil((float)ThreadGroupCountY / (float)DispatchGroupSize.y), glm::ceil((float)ThreadGroupCountZ / (float)DispatchGroupSize.z));
	}
	else
	{
		ensureMsgf(false, "No shader bound");
		Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
	}
}

void RHICommandList::SetCommandSigniture(RHICommandSignitureDescription desc)
{}
