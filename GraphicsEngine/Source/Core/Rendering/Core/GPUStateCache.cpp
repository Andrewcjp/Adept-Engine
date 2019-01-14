#include "GPUStateCache.h"
#define GPUCACHE_DISABLED 1
GPUStateCache::GPUStateCache()
{}


GPUStateCache::~GPUStateCache()
{}

bool GPUStateCache::RenderTargetCheckAndUpdate(FrameBuffer * RT)
{
#if GPUCACHE_DISABLED
	return false;
#endif
	if (RenderTarget == RT)
	{
		return true;
	}
	RenderTarget = RT;
	return false;
}

bool GPUStateCache::TextureCheckAndUpdate(BaseTexture * tex, int slot)
{
#if GPUCACHE_DISABLED
	return false;
#endif
	if (Textures[slot] == tex)
	{
		return true;
	}
	Textures[slot] = tex;
	return false;
}
