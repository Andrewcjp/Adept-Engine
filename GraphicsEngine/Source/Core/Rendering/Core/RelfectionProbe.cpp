
#include "RelfectionProbe.h"
#include "RHI/RHI_inc.h"

RelfectionProbe::RelfectionProbe()
{
	RHIFrameBufferDesc CubeDesc;
	CubeDesc = RHIFrameBufferDesc::CreateColourDepth(Dimentions.x, Dimentions.y);
	CubeDesc.Dimension = eTextureDimension::DIMENSION_TEXTURECUBE;
	CubeDesc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	CubeDesc.TextureDepth = 6;
	CubeDesc.MipCount = 11;
	/*CubeDesc.DepthClearValue = 1.0f;*/
	CubeDesc.CubeMapAddressAsOne = false;
	CubeDesc.AllowUnordedAccess = false;
	CapturedTexture = RHI::CreateFrameBuffer(RHI::GetDefaultDevice(), CubeDesc);
	CapturedTexture->SetDebugName("RelfectionProbe");
}


RelfectionProbe::~RelfectionProbe()
{
	EnqueueSafeRHIRelease(CapturedTexture);
}
