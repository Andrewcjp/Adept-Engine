#pragma once
#include "VKanRHI.h"
class VKanRenderPass :public RHIRenderPass
{
public:
	VKanRenderPass(RHIRenderPassDesc & desc, DeviceContext* Device);
	virtual void Complie() override;
	VkRenderPass RenderPass = VK_NULL_HANDLE;
	VkanDeviceContext* VDevice = nullptr;
};

