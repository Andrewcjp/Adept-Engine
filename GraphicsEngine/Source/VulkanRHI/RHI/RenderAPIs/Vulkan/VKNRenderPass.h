#pragma once
#include "VKNRHI.h"
class VKNRenderPass :public RHIRenderPass
{
public:
	VKNRenderPass(RHIRenderPassDesc & desc, DeviceContext* Device);
	virtual void Complie() override;
	VkRenderPass RenderPass = VK_NULL_HANDLE;
	VKNDeviceContext* VDevice = nullptr;
};

