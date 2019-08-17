#pragma once
#include "RHI\Streaming\GPUTextureStreamer.h"

class D3D12DeviceContext;
class DXGPUTextureStreamer : public GPUTextureStreamer
{
public:
	DXGPUTextureStreamer();
	~DXGPUTextureStreamer();
protected:
	virtual void Tick() override;
	virtual void OnInit(DeviceContext* con) override;
private:
	D3D12DeviceContext* Device = nullptr;
};

