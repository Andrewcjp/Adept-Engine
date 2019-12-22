#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
Texture2D<uint> RateImage: register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
cbuffer Data : register(b1)
{
	int2 Resolution;
	bool DebugShow;
	bool DebugShowLines;
};
float4 GetColourForRate(int r)
{
	int2 rate = GetShadingRate(r);
	int totalrate = max(rate.x,rate.y);
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
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	int2 ShadingRateImageXY = (DTid.xy / 16);
	int ShadingRate = RateImage[ShadingRateImageXY.xy];
	int2 Rate = GetShadingRate(ShadingRate);
	if (!IsPixelSource(DTid.xy, Rate))
	{
		//find the corse pixel for this pixel
		const int2 DeltaToMain = DTid.xy % Rate.xy;
		int2 SourcePixel = DTid.xy - DeltaToMain;
		DstTexture[DTid.xy] = DstTexture[SourcePixel];	
		//todo: use other corse pixels to smooth output
	}
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
}



