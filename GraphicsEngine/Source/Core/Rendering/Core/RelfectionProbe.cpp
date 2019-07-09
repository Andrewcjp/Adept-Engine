
#include "RelfectionProbe.h"
#include "RHI/RHI_inc.h"

RelfectionProbe::RelfectionProbe()
{
	ProbeMode = EReflectionProbeMode::ERealTime;

	int size = 1024;
	Dimentions.x = size;
	Dimentions.y = size;
	RHIFrameBufferDesc CubeDesc;
	CubeDesc = RHIFrameBufferDesc::CreateColourDepth(Dimentions.x, Dimentions.y);
	CubeDesc.Dimension = eTextureDimension::DIMENSION_TEXTURECUBE;
	CubeDesc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	CubeDesc.DepthFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	CubeDesc.TextureDepth = 6;
	CubeDesc.MipCount = 9;
	/*CubeDesc.DepthClearValue = 1.0f;*/
	CubeDesc.CubeMapAddressAsOne = false;
	CubeDesc.AllowUnorderedAccess = true;
	for (int d = 0; d < CubeDesc.TextureDepth; d++)
	{
		for (int i = 0; i < CubeDesc.MipCount; i++)
		{
			RHIViewDesc Desc;
			Desc.Mip = i;
			Desc.Dimention = DIMENSION_TEXTURE2DARRAY;
			Desc.Slice = d;
			CubeDesc.RequestedViews.push_back(Desc);
		}
	}
	CapturedTexture = RHI::CreateFrameBuffer(RHI::GetDefaultDevice(), CubeDesc);
	CapturedTexture->SetDebugName("RelfectionProbe");

	ConvolutionRes = 64;
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateCubeColourDepth(ConvolutionRes, ConvolutionRes);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	ConvolutionBuffer = RHI::CreateFrameBuffer(RHI::GetDefaultDevice(), Desc);
}


RelfectionProbe::~RelfectionProbe()
{
	EnqueueSafeRHIRelease(CapturedTexture);
}

IntPoint RelfectionProbe::GetDimentions() const
{
	return Dimentions;
}

bool RelfectionProbe::NeedsCapture() const
{
	if (ProbeMode == EReflectionProbeMode::Baked)
	{
		return !IsCaptured;
	}
	return true;
}

void RelfectionProbe::SetCaptured()
{
	IsCaptured = true;
}

bool RelfectionProbe::GetNeedsDownSample() const
{
	return NeedsDownSample;
}

void RelfectionProbe::SetNeedsDownSample(bool val)
{
	NeedsDownSample = val;
}
