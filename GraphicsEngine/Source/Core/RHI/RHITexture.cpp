#include "RHITexture.h"


RHITexture::RHITexture()
{}


RHITexture::~RHITexture()
{}

const RHITextureDesc2 & RHITexture::GetDescription()const
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

eTEXTURE_FORMAT RHITextureDesc2::GetRenderformat() const
{
	if (RenderFormat != eTEXTURE_FORMAT::FORMAT_UNKNOWN)
	{
		return RenderFormat;
	}
	return Format;
}

eTEXTURE_FORMAT RHITextureDesc2::GetDepthRenderformat() const
{
	if (DepthRenderFormat != eTEXTURE_FORMAT::FORMAT_UNKNOWN)
	{
		return DepthRenderFormat;
	}
	return Format;
}