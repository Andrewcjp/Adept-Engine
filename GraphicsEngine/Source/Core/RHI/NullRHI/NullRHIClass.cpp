
#include "NullRHIClass.h"
#include "NullRHIDeviceContext.h"
#include "NullRHIShaderProgram.h"
#include "NullRHIBuffer.h"

bool NullRHIClass::InitRHI(bool InitAllGPUs)
{
	Dev = new NullRHIDeviceContext();
	return true;
}

bool NullRHIClass::InitWindow(int w, int h)
{
	return false;
}

bool NullRHIClass::DestoryRHI()
{
	return true;
}

BaseTexture* NullRHIClass::CreateTexture(const RHITextureDesc& Desc, DeviceContext* Device /*= nullptr*/)
{
	return nullptr;
}

FrameBuffer* NullRHIClass::CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc)
{
	return nullptr;
}

ShaderProgramBase* NullRHIClass::CreateShaderProgam(DeviceContext* Device /*= nullptr*/)
{
	return new NullRHIShaderProgram(Dev);
}

RHITextureArray * NullRHIClass::CreateTextureArray(DeviceContext * Device, int Length)
{
	return nullptr;
}

RHIBuffer* NullRHIClass::CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext* Device /*= nullptr*/)
{
	return new NullRHIBuffer();
}

RHICommandList* NullRHIClass::CreateCommandList(ECommandListType::Type Type /*= ECommandListType::Graphics*/, DeviceContext* Device /*= nullptr*/)
{
	return nullptr;
}

DeviceContext* NullRHIClass::GetDefaultDevice()
{
	return Dev;
}

DeviceContext* NullRHIClass::GetDeviceContext(int index /*= 0*/)
{
	return Dev;
}

void NullRHIClass::RHISwapBuffers()
{
	
}

void NullRHIClass::RHIRunFirstFrame()
{
	
}

void NullRHIClass::SetFullScreenState(bool state)
{
	
}

void NullRHIClass::ResizeSwapChain(int width, int height)
{
	
}

void NullRHIClass::WaitForGPU()
{
	
}

void NullRHIClass::TriggerBackBufferScreenShot()
{
	
}

RHIClass::GPUMemoryReport NullRHIClass::ReportMemory()
{
	return RHIClass::GPUMemoryReport();
}

RHIPipeLineStateObject* NullRHIClass::CreatePSO(const RHIPipeLineStateDesc& Desc, DeviceContext * Device)
{
	return nullptr;
}

RHIGPUSyncEvent* NullRHIClass::CreateSyncEvent(EDeviceContextQueue::Type WaitingQueue, EDeviceContextQueue::Type SignalQueue, DeviceContext * Device, DeviceContext * SignalDevice)
{
	return nullptr;
}

RHIQuery * NullRHIClass::CreateQuery(EGPUQueryType::Type type, DeviceContext * con)
{
	return nullptr;
}

RHIRenderPass* NullRHIClass::CreateRenderPass(const RHIRenderPassDesc & Desc, DeviceContext* Device)
{
	return nullptr;
}

RHIInterGPUStagingResource* NullRHIClass::CreateInterGPUStagingResource(DeviceContext* Owner, const InterGPUDesc& desc)
{
	return nullptr;
}

RHITexture* NullRHIClass::CreateTexture2()
{
	return nullptr;
}

void NullRHIClass::MakeSwapChainReady(RHICommandList* list)
{
	
}

RHIFence* NullRHIClass::CreateFence(DeviceContext* device, EFenceFlags::Type Flags /*= EFenceFlags::None*/)
{
	return nullptr;
}
