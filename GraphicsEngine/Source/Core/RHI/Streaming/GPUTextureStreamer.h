#pragma once
#include "TextureStreamingEngine.h"
#include "TextureStreamingCommon.h"

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
	virtual void SetStreamingMode(EGPUSteamMode::Type mode);
protected:
	EGPUSteamMode::Type m_StreamingMode = EGPUSteamMode::TiledTexture;
	uint64 TargetPoolSize = 0;
	RHI_API virtual void OnRealiseTexture(TextureHandle* handle) {};
	RHI_API virtual void Tick(RHICommandList* list);
	RHI_API virtual void OnInit(DeviceContext* con);
	std::vector<TextureHandle*> Handles;
	RHICommandList* CmdList = nullptr;
};

