#pragma once
#include "RHI\Streaming\GPUTextureStreamer.h"

class D3D12DeviceContext;
class GPUMemoryPage;
class DXGPUTextureStreamer : public GPUTextureStreamer
{
public:
	DXGPUTextureStreamer();
	~DXGPUTextureStreamer();

	void SetStreamingMode(EGPUSteamMode::Type mode) override;

protected:
	virtual void Tick(RHICommandList* list) override;
	virtual void OnInit(DeviceContext* con) override;

	virtual void OnRealiseTexture(TextureHandle* handle) override;

	void MapHandle(TextureHandle* handle,RHICommandList* list);

private:
	D3D12DeviceContext* Device = nullptr;
	GPUMemoryPage* Page = nullptr;
};

