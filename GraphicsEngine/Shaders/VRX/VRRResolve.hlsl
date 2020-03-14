#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
#include "Utils/UavLoad.hlsl"
#if 0 //!FULL_UAV_LOAD
Texture2D<float4> SrcTexture: register(t3);
#endif
Texture2D<uint> RateImage: register(t0);
SAFEUAVLOAD DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
StructuredBuffer<uint4> TileList : register(t2);
cbuffer Data : register(b1)
{
	int2 Resolution;
	float LerpBlend;
	bool DebugShow;
	bool DebugShowLines;
};


uint2 GetCoursePixelPos(uint2 Fullpos,out float dist)
{
	const uint ShadingRate = RateImage[Fullpos.xy / VRS_TILE_SIZE];
	const uint2 Rate = GetShadingRate(ShadingRate);
	if (!IsPixelSource(Fullpos.xy, Rate))
	{
		const int2 DeltaToMain = Fullpos % Rate.xy;
		const int2 SourcePixel = Fullpos - DeltaToMain;
		dist = abs((Rate.x+Rate.y)-length(DeltaToMain));
		dist = clamp(dist, 0, 1);
		//dist = 1.0f;
		return SourcePixel;
	}
	dist = 1.0f;
	return Fullpos.xy;
}

#define VRR_BLEND 1
#define BUILD_SHIPPING 0
#ifndef PS_RESOLVE
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
#if  0//VRR_BLEND
		[branch]
		if (LerpBlend > 0.0f)
		{
			float Distance[4];
			const float4 Sample1 = DstTexture[GetCoursePixelPos(Pixel.xy + int2(Rate.x, 0), Distance[0])];
			const float4 Sample2 = DstTexture[GetCoursePixelPos(Pixel.xy + int2(0, Rate.y), Distance[1])];
			const float4 Sample3 = DstTexture[GetCoursePixelPos(Pixel.xy - int2(Rate.x, 0), Distance[2])];
			const float4 Sample4 = DstTexture[GetCoursePixelPos(Pixel.xy - int2(0, Rate.y), Distance[3])];
			float4 AVg = (Sample1 + Sample2  + Sample3 + Sample4 ) / 4;
			const float blend = DstTexture[Pixel.xy].r;
			DstTexture[Pixel.xy] =  lerp(DstTexture[SourcePixel], AVg, 1.0-blend);
		}
		else
#endif
		{
			WriteUAV(LoadUAV(SourcePixel, DstTexture), Pixel.xy, DstTexture);
		}
	}
}
#endif


