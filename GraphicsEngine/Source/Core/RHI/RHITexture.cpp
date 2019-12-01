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

void RHITexture::CreateWithUpload(const TextureDescription & idesc, DeviceContext* iContext)
{
	UploadDesc = idesc;

}


void RHITexture::CopyToStagingResource(RHIInterGPUStagingResource* Res, RHICommandList* List)
{


}

void RHITexture::CopyFromStagingResource(RHIInterGPUStagingResource * Res, RHICommandList * list)
{

}
