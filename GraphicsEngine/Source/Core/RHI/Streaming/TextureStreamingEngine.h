#pragma once
#include "GPUTextureStreamer.h"
#include "TextureStreamingCommon.h"

//Manages all texture streaming
//Launches requests to GPU texture steamers on each GPU.
class TextureStreamingEngine
{
public:
	TextureStreamingEngine();
	~TextureStreamingEngine();
	//Updates the streaming engines
	void Update();
	//Determines what textures are needed this frame/ next.
	void UpdateSceneTextureStreaming();
	static TextureStreamingEngine* Get()
	{
		return Instance;
	}
	static void ShutDown()
	{
		SafeDelete(Instance);
	}
	void RegisterGPUStreamer(GPUTextureStreamer* Stream);
	static TextureHandle* RequestTexture(std::string File);
	
	EGPUSteamMode::Type GetStreamingMode() const { return m_StreamingMode; }
	void SetStreamingMode(EGPUSteamMode::Type val) { m_StreamingMode = val; }
	void ResolveTextureStreaming(RHICommandList* list);
private:
	EGPUSteamMode::Type m_StreamingMode = EGPUSteamMode::TiledTexture;
	uint64_t TargetMaxMemory = 0;
	static TextureStreamingEngine* Instance;
	std::vector<GPUTextureStreamer*> Streamers;
	std::queue<TextureHandle*> NewHandles;
	//todo: map?
	std::vector<TextureHandle*> Handles;
};

