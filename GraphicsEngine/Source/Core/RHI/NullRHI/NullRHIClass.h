#pragma once
#include "RHI/RHI.h"

class NullRHIDeviceContext;
class NullRHIClass : public RHIClass
{

public:
	RHI_VIRTUAL bool InitRHI(bool InitAllGPUs) override;
	RHI_VIRTUAL bool InitWindow(int w, int h) override;
	RHI_VIRTUAL bool DestoryRHI() override;
	RHI_VIRTUAL BaseTexture* CreateTexture(const RHITextureDesc& Desc, DeviceContext* Device = nullptr) override;
	RHI_VIRTUAL FrameBuffer* CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc) override;
	RHI_VIRTUAL ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr) override;
	RHI_VIRTUAL RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length) override;
	RHI_VIRTUAL RHIBuffer* CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext* Device = nullptr) override;
	RHI_VIRTUAL RHICommandList* CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext* Device = nullptr) override;
	RHI_VIRTUAL DeviceContext* GetDefaultDevice() override;
	RHI_VIRTUAL DeviceContext* GetDeviceContext(int index = 0) override;
	RHI_VIRTUAL void RHISwapBuffers() override;
	RHI_VIRTUAL void RHIRunFirstFrame() override;
	RHI_VIRTUAL void SetFullScreenState(bool state) override;
	RHI_VIRTUAL void ResizeSwapChain(int width, int height) override;
	RHI_VIRTUAL void WaitForGPU() override;
	RHI_VIRTUAL void TriggerBackBufferScreenShot() override;
	RHI_VIRTUAL RHIClass::GPUMemoryReport ReportMemory() override;
	RHI_VIRTUAL RHIPipeLineStateObject* CreatePSO(const RHIPipeLineStateDesc& Desc, DeviceContext * Device) override;
	RHI_VIRTUAL RHIGPUSyncEvent* CreateSyncEvent(EDeviceContextQueue::Type WaitingQueue, EDeviceContextQueue::Type SignalQueue, DeviceContext * Device, DeviceContext * SignalDevice) override;
	RHI_VIRTUAL RHIQuery * CreateQuery(EGPUQueryType::Type type, DeviceContext * con) override;
	RHI_VIRTUAL RHIRenderPass* CreateRenderPass(const RHIRenderPassDesc & Desc, DeviceContext* Device) override;
	RHI_VIRTUAL RHIInterGPUStagingResource* CreateInterGPUStagingResource(DeviceContext* Owner, const InterGPUDesc& desc) override;
	RHI_VIRTUAL RHITexture* CreateTexture2() override;
	RHI_VIRTUAL void MakeSwapChainReady(RHICommandList* list) override;
	RHI_VIRTUAL RHIFence* CreateFence(DeviceContext* device, EFenceFlags::Type Flags = EFenceFlags::None) override;
private:
	NullRHIDeviceContext* Dev = nullptr;
};

