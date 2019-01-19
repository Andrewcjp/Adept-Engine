#pragma once

#include "RHI/RHITypes.h"
class FrameBuffer : public IRHIResourse
{
public:
	RHI_API FrameBuffer(class DeviceContext* device, const RHIFrameBufferDesc& Desc);
	RHI_API virtual ~FrameBuffer();

	int GetWidth() const
	{
		return m_width;
	}
	int GetHeight() const
	{
		return m_height;
	}
	virtual class DeviceContext* GetDevice() = 0;
	void HandleInit();
	RHI_API RHIFrameBufferDesc& GetDescription();
	void Resize(int width, int height);
	RHI_API virtual void HandleResize();

	int m_width = 0;
	int m_height = 0;
	virtual void SetupCopyToDevice(DeviceContext* device)
	{};
	static void CopyHelper(FrameBuffer* Target, DeviceContext* TargetDevice);
	static void CopyHelper_Async_OneFrame(FrameBuffer * Target, DeviceContext * TargetDevice);
	static void CopyHelper_Async(FrameBuffer * Target, DeviceContext * TargetDevice);
	virtual const RHIPipeRenderTargetDesc& GetPiplineRenderDesc() = 0;
	RHI_API virtual void BindDepthWithColourPassthrough(class RHICommandList* list, FrameBuffer* PassThrough);
	
	virtual void MakeReadyForComputeUse(RHICommandList* List) = 0;
	virtual void MakeReadyForCopy(RHICommandList * list) = 0;
	///Needs to called before buffer is read for final present
	RHI_API virtual void ResolveSFR();
protected:
	RHIFrameBufferDesc BufferDesc;
	SFRNode* SFR_Node = nullptr;
	DeviceContext* Device = nullptr;
};

