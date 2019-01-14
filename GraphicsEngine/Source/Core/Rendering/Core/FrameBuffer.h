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
	RHI_API RHIFrameBufferDesc& GetDescription();
	RHI_API virtual void Resize(int width, int height);

	int m_width = 0;
	int m_height = 0;
	virtual void SetupCopyToDevice(DeviceContext* device)
	{};
	static void CopyHelper(FrameBuffer* Target, DeviceContext* TargetDevice);
	virtual const RHIPipeRenderTargetDesc& GetPiplineRenderDesc() = 0;
	RHI_API virtual void BindDepthWithColourPassthrough(class RHICommandList* list, FrameBuffer* PassThrough);
	virtual void MakeReadyForComputeUse(RHICommandList* List) = 0;
	virtual void MakeReadyForCopy(RHICommandList * list) = 0;
protected:
	RHIFrameBufferDesc BufferDesc;

};

