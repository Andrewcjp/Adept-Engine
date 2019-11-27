#include "RHITexture.h"


RHITexture::RHITexture()
{}


RHITexture::~RHITexture()
{}

const RHITextureDesc2 & RHITexture::GetDescription()
{
	return Desc;
}

void RHITexture::Create(const RHITextureDesc2 & iDesc, DeviceContext* iContext)
{}
