#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
Texture2D<uint> RateImage: register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);

cbuffer Data : register(b1)
{
	int2 Resolution;
	float LerpBlend;
	bool DebugShow;
	bool DebugShowLines;
};
float4 GetColourForRate(int r)
{
	const int2 rate = GetShadingRate(r);
	const int totalrate = max(rate.x, rate.y);
	if (totalrate == 1)
	{
		return float4(1, 0, 0, 0);
	}
	else if (totalrate == 2)
	{
		return float4(0, 1, 0, 0);
	}
	else if (totalrate == 4)
	{
		return float4(0, 0, 1, 0);
	}
	return float4(1, 1, 1, 0);
}

float4 SampleCoursePixel(uint2 Fullpos)
{
	const int ShadingRate = RateImage[Fullpos.xy / VRS_TILE_SIZE];
	const int2 Rate = GetShadingRate(ShadingRate);
	if (!IsPixelSource(Fullpos.xy, Rate))
	{
		const int2 DeltaToMain = Fullpos % Rate.xy;
		const int2 SourcePixel = Fullpos - DeltaToMain;
		return DstTexture[SourcePixel];
	}
	return DstTexture[Fullpos.xy];
}
#define VRR_BLEND 1
#define BUILD_SHIPPING 0
#ifndef PS_RESOLVE
float avg_intensity(in float3 pix)
{
	return (pix.r + pix.g + pix.b) / 3.;
}
float threshold(in float thr1, in float thr2, in float val)
{
	if (val < thr1) { return 0.0; }
	if (val > thr2) { return 1.0; }
	return val;
}
float3 get_pixel(in uint2 coords, in int dx, in int dy)
{
	return DstTexture[coords + uint2(dx, dy)];
}
float IsEdge(uint2 coords)
{
	float pix[9];
	int k = -1;
	float delta;

	// read neighboring pixel intensities
	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			k++;
			pix[k] = avg_intensity(get_pixel(coords, (i), (j)));
		}
	}

	// average color differences around neighboring pixels
	delta = (abs(pix[1] - pix[7]) +
		abs(pix[5] - pix[3]) +
		abs(pix[0] - pix[8]) +
		abs(pix[2] - pix[6])
		) / 4.;

	return threshold(0.0,1.0,clamp(delta, 0.0, 1.0));
}
bool DetectEdgeAtPX(uint2 px)
{
	return (IsEdge(px) > 0.0);
}
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	const int2 ShadingRateImageXY = (DTid.xy / VRS_TILE_SIZE);
	const int ShadingRate = RateImage[ShadingRateImageXY.xy];
	uint2 Rate = GetShadingRate(ShadingRate);

	[branch]
	if (!IsPixelSource(DTid.xy, Rate))
	{
		//find the corse pixel for this pixel
		const int2 DeltaToMain = DTid.xy % Rate.xy;
		const int2 SourcePixel = DTid.xy - DeltaToMain;
#if VRR_BLEND
		[branch]
		if (LerpBlend > 0.0f)
		{
			float4 AVg = SampleCoursePixel(DTid.xy + int2(Rate.x, 0));
			AVg += SampleCoursePixel(DTid.xy + int2(0, Rate.y));
			AVg += SampleCoursePixel(DTid.xy - int2(Rate.x, 0));
			AVg += SampleCoursePixel(DTid.xy + int2(0, -Rate.y));
			AVg /= 4;
			DstTexture[DTid.xy] = lerp(DstTexture[SourcePixel], AVg, LerpBlend);
		}
		else
#endif
		{
			DstTexture[DTid.xy] = DstTexture[SourcePixel];
		}
		//todo: use other corse pixels to smooth output
	}

#if 0
	float4 outt = float4(0, 0, 0, 0);// DstTexture[DTid.xy];
	outt.r = IsEdge(DTid.xy);
	DstTexture[DTid.xy] = outt;
#endif
#if !BUILD_SHIPPING
	if (DebugShow)
	{
		DstTexture[DTid.xy] += GetColourForRate(ShadingRate)*0.5f;
	}
	if (DebugShowLines)
	{
		if ((DTid.x % VRS_TILE_SIZE == 0) || (DTid.y % VRS_TILE_SIZE == 0))
		{
			DstTexture[DTid.xy] += float4(1, 0, 0, 1)*0.5f;
		}
	}
#endif
}
#endif


