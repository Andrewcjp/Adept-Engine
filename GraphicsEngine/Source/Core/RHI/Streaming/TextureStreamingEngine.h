#pragma once
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
private:
	uint64_t TargetMaxMemory = 0;

};

