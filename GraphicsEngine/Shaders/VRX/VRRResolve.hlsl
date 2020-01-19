#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
Texture2D<uint> RateImage: register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
StructuredBuffer<uint4> TileList : register(t2);
cbuffer Data : register(b1)
{
	int2 Resolution;
	float LerpBlend;
	bool DebugShow;
	bool DebugShowLines;
};
float4 GetColourForRate(uint2 rate)
{
	const int totalrate = rate.x + rate.y;
	if (totalrate == 2)
	{
		return float4(1, 0, 0, 0);
	}
	else if (totalrate == 3)
	{
		return float4(1, 1, 0, 0);
	}
	else if (totalrate == 4)
	{
		return float4(0, 1, 0, 0);
	}
	else if (totalrate == 6)
	{
		return float4(1, 0, 1, 0);
	}
	else if (totalrate == 8)
	{
		return float4(0, 0, 1, 0);
	}
	return float4(1, 1, 1, 0);
}

uint2 GetCoursePixelPos(uint2 Fullpos)
{
	const uint ShadingRate = RateImage[Fullpos.xy / VRS_TILE_SIZE];
	const uint2 Rate = GetShadingRate(ShadingRate);
	if (!IsPixelSource(Fullpos.xy, Rate))
	{
		const int2 DeltaToMain = Fullpos % Rate.xy;
		const int2 SourcePixel = Fullpos - DeltaToMain;
		return SourcePixel;
	}
	return Fullpos.xy;
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

	return threshold(0.0, 1.0, clamp(delta, 0.0, 1.0));
}
bool DetectEdgeAtPX(uint2 px)
{
	return (IsEdge(px) > 0.0);
}
[numthreads(VRS_TILE_SIZE, VRS_TILE_SIZE, 1)]
void main(uint3 DTid : SV_GroupThreadID, uint3 groupIndex : SV_GroupID)
{
	const uint4 TileData = TileList[groupIndex.x];
	uint2 Pixel = TileData.xy * VRS_TILE_SIZE;
	const uint2 Rate = TileData.zw;

	Pixel += DTid.xy;

	[branch]
	if (!IsPixelSource(Pixel.xy, Rate))
	{
		//find the corse pixel for this pixel
		const int2 DeltaToMain = Pixel.xy % Rate.xy;
		const int2 SourcePixel = Pixel.xy - DeltaToMain;
#if VRR_BLEND
		[branch]
		if (LerpBlend > 0.0f)
		{
			const float4 Sample1 = DstTexture[GetCoursePixelPos(Pixel.xy + int2(Rate.x, 0))];
			const float4 Sample2 = DstTexture[GetCoursePixelPos(Pixel.xy + int2(0, Rate.y))];
			const float4 Sample3 = DstTexture[GetCoursePixelPos(Pixel.xy - int2(Rate.x, 0))];
			const float4 Sample4 = DstTexture[GetCoursePixelPos(Pixel.xy + int2(0, -Rate.y))];
			const float4 AVg = (Sample1 + Sample2 + Sample3 + Sample4) / 4;
			DstTexture[Pixel.xy] = lerp(DstTexture[SourcePixel], AVg, LerpBlend);
		}
		else
#endif
		{
			DstTexture[Pixel.xy] = DstTexture[SourcePixel];
		}
	}

#if !BUILD_SHIPPING
	if (DebugShow)
	{
		DstTexture[Pixel.xy] += GetColourForRate(Rate)*0.5f;
	}
	if (DebugShowLines)
	{
		if ((Pixel.x % VRS_TILE_SIZE == 0) || (Pixel.y % VRS_TILE_SIZE == 0))
		{
			DstTexture[Pixel.xy] += float4(1, 0, 0, 1)*0.5f;
		}
	}
#endif
}
#endif


