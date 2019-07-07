#pragma once
#include "RHI\RHIInterGPUStagingResource.h"
class D3D12InterGPUStagingResource : public RHIInterGPUStagingResource
{
public:
	D3D12InterGPUStagingResource(DeviceContext* owner);
	~D3D12InterGPUStagingResource();
};

