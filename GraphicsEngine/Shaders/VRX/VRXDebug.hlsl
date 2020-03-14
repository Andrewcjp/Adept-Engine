#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
Texture2D<uint> RateImage: register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
 RWByteAddressBuffer PixelCount:register(u1);
cbuffer Data : register(b1)
{
	int2 Resolution;
	float LerpBlend;
	bool DebugShow;
	bool DebugShowLines;
	bool EdgeHeat;
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

[numthreads(VRS_TILE_SIZE, VRS_TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	if (DTid.x == 0 && DTid.y == 0)
	{
		uint value;
		PixelCount.InterlockedExchange(0, 0, value);		
	}
	uint2 Rate = GetShadingRate(RateImage[DTid.xy / VRS_TILE_SIZE]);

	if (DebugShow)
	{
		DstTexture[DTid.xy] += GetColourForRate(Rate)*0.5f;
	}
	if (DebugShowLines)
	{
		if ((DTid.x % VRS_TILE_SIZE == 0) || (DTid.y % VRS_TILE_SIZE == 0))
		{
			DstTexture[DTid.xy] += float4(1, 0, 0, 1)*0.5f;
		}
	}
	if (EdgeHeat)
	{
		float Scale = (float)RateImage[DTid.xy / VRS_TILE_SIZE] / (float)255;
		float3 heat = lerp(float3(1, 0, 0), float3(0, 0, 1), 1.0 - saturate(Scale));
		DstTexture[DTid.xy]  = float4(heat, 1);
	}
	if (IsPixelSource(DTid.xy, Rate))
	{
		//todo don't count invalid pixels
		uint value = 0;
		PixelCount.InterlockedAdd(0, 1, value);
	}
}