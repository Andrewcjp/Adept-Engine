#pragma once
#include "RHI/RHITypes.h"

class DeviceContext;
class RHIGPUSyncEvent;
class RHICommandList;
class RHIUAV;
class RHIInterGPUStagingResource;
class FrameBuffer : public IRHIResourse
{
public:
	RHI_API FrameBuffer(DeviceContext* device, const RHIFrameBufferDesc& Desc);
	RHI_API virtual ~FrameBuffer();
	RHI_API int GetWidth() const;
	RHI_API int GetHeight() const;
	virtual DeviceContext* GetDevice() = 0;

	RHI_API RHIFrameBufferDesc& GetDescription();
	void Resize(int width, int height);
	RHI_API void SFRResize();

	static void CopyHelper(FrameBuffer* Target, DeviceContext* TargetDevice, EGPUCOPYTIMERS::Type Stat = EGPUCOPYTIMERS::MGPUCopy, DeviceContextQueue::Type CopyQ = DeviceContextQueue::Copy);
	RHI_API virtual void CopyToOtherBuffer(FrameBuffer * OtherBuffer, RHICommandList* List);
	static void CopyHelper_Async_OneFrame(FrameBuffer * Target, DeviceContext * TargetDevice);
	static void CopyHelper_Async(FrameBuffer * Target, DeviceContext * TargetDevice);
	virtual const RHIPipeRenderTargetDesc& GetPiplineRenderDesc() = 0;
	RHI_API virtual void BindDepthWithColourPassthrough(class RHICommandList* list, FrameBuffer* PassThrough);
	virtual void MakeReadyForComputeUse(RHICommandList* List, bool Depth = false) = 0;
	virtual void MakeReadyForCopy(RHICommandList * list) = 0;
	///Needs to called before buffer is read for final present
	void ResolveSFR(FrameBuffer* SumBuffer);
	int GetTransferSize();
	void ResetTransferStat();
	static void CopyHelper_NewSync(FrameBuffer * Target, DeviceContext * TargetDevice, EGPUCOPYTIMERS::Type Stat, DeviceContextQueue::Type CopyQ = DeviceContextQueue::Copy);
	void BindUAV(RHICommandList* list,int slot, int UAVIndex = 0);
	RHI_API RHIUAV* GetUAV();
	

	RHI_API virtual void Release() override;


	RHI_API virtual size_t GetSizeOnGPU() override;
	static void AutoUpdateSize(RHIFrameBufferDesc & desc);
	//Helper that uses the SizeMode in the desc to resize FB
	void AutoResize();

	//creates a SRV for this buffer and stores it in its cache.
	virtual void RequestSRV(const RHIViewDesc & desc) {};
	RHIUAV* GetUAV(const RHIViewDesc & desc);

	RHI_API virtual void CopyToStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List);
	RHI_API virtual void CopyFromStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* list);
protected:
	RHI_API virtual void HandleResize();
	void SetupFences();

	void HandleInit();
	RHI_API void PostInit();
	int CrossGPUBytes = 0;
	bool DidTransferLastFrame = false;
	int m_width = 0;
	int m_height = 0;
	RHI_API bool NeedsSFRResolve() const;
	RHIFrameBufferDesc BufferDesc;
	SFRNode* SFR_Node = nullptr;
	DeviceContext* Device = nullptr;
	RHIGPUSyncEvent* CopyFence = nullptr;
	RHIGPUSyncEvent* DeviceFence = nullptr;
	RHIGPUSyncEvent* TargetCopyFence = nullptr;
	
	
	std::vector<RHIUAV*> UAVs;
};

