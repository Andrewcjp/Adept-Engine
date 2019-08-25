#define SUPPORT_VRR 1
#include "VRX/VRSCommon.hlsl"
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
	if (r == 1)
	{
		return float4(1, 0, 0, 0);
	}
	else if (r == 2)  
	{
		return float4(0, 1, 0, 0);
	}
	else if (r == 3)
	{
		return float4(0, 0, 1, 0);
	}
	return float4(0, 0, 0, 0);
}
[numthreads(4, 4, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float2 ScreenUV = (float2)DTid.xy / (float2)Resolution;
	int ShadingRate = GetShadingRateIDForPixel(ScreenUV, Resolution); 
	if (!IsPixelSource(DTid.xy, GetShadingRate(ShadingRate)))
	{
		//find the corse pixel for this pixel
		const int2 DeltaToMain = DTid.xy % GetShadingRate(ShadingRate).xy;
		int2 SourcePixel = DTid.xy - DeltaToMain;
		//if (DstTexture[DTid.xy].a < 0.5)
		{
			DstTexture[DTid.xy] = DstTexture[SourcePixel];
		}
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



