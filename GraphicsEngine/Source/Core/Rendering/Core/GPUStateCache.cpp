#include "GPUStateCache.h"

GPUStateCache::GPUStateCache()
{}


GPUStateCache::~GPUStateCache()
{}

bool GPUStateCache::RenderTargetCheckAndUpdate(FrameBuffer * RT)
{
	if (RenderTarget == RT)
	{
		return true;
	}
	RenderTarget = RT;
	return false;
}

bool GPUStateCache::TextureCheckAndUpdate(BaseTexture * tex, int slot)
{
	if (Textures[slot] == tex)
	{
		return true;
	}
	Textures[slot] = tex;
	return false;
}
