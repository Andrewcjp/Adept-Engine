#pragma once

class TextureHandle;
//Textures need to be registered against this
class GPUTextureStreamer
{
public:
	RHI_API GPUTextureStreamer();
	RHI_API virtual ~GPUTextureStreamer();
	RHI_API void Init(DeviceContext* Con);
	void Update();
	void RealiseHandle(TextureHandle* handle);
	void SetTargetSize(uint64 size);
protected:
	uint64 TargetPoolSize = 0;
	RHI_API virtual void OnRealiseTexture(TextureHandle* handle) {};
	RHI_API virtual void Tick(RHICommandList* list);
	RHI_API virtual void OnInit(DeviceContext* con);
	std::vector<TextureHandle*> Handles;
	RHICommandList* list = nullptr;
};

