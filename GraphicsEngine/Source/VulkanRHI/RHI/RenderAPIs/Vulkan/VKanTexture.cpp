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
void VKanTexture::CreateTextureFromData(void * data, int type, int width, int height, int bits)
{}
void VKanTexture::CreateAsNull()
{}

void VKanTexture::UpdateSRV()
{}

#endif