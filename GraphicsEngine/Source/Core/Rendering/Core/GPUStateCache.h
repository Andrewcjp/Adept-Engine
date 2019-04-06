#pragma once
//#RHI Integrate correctly 
class RHIPipeLineStateObject;
class FrameBuffer;
class BaseTexture;
#define MAX_MRTS 8
class GPUStateCache
{
public:
	GPUStateCache();	
	~GPUStateCache();

	CORE_API bool RenderTargetCheckAndUpdate(FrameBuffer* RT);
	CORE_API bool TextureCheckAndUpdate(BaseTexture* tex, int slot);
private:
	BaseTexture* Textures[50];//todo: how does this work with pipeline state objects?
	FrameBuffer* RenderTarget;
};

