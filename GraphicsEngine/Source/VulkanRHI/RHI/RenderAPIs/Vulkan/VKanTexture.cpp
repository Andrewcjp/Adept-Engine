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

void VKanTexture::CreateAsNull()
{}

void VKanTexture::UpdateSRV()
{

}

void VKanTexture::CreateTextureFromDesc(const TextureDescription& desc)
{

}

#endif