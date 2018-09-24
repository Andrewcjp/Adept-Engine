#include "stdafx.h"
#include "VKanTexture.h"

#if BUILD_VULKAN

VKanTexture::VKanTexture()
{}


VKanTexture::~VKanTexture()
{}
bool VKanTexture::CreateFromFile(AssetPathRef FileName)
{
	return true;
}
void VKanTexture::CreateTextureAsRenderTarget(int width, int height, bool depthonly, bool alpha)
{}
void VKanTexture::CreateTextureFromData(void * data, int type, int width, int height, int bits)
{}
void VKanTexture::CreateAsNull()
{}
#endif