#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
Texture2D<uint> RateImage : register(t0);
SamplerState defaultSampler : register (s0);
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
PUSHCONST cbuffer ResData : register(b1)
{
	int2 Resolution;
};
float4 main(VS_OUTPUT input) : SV_Target
{
	int2 pos = input.uv.xy *(float2)Resolution.xy;
	int ShadingRate = RateImage[pos.xy/ VRS_TILE_SIZE];
	int2 Rate =  GetShadingRate(ShadingRate);
	if (IsPixelSource(pos.xy, Rate))
	{
		discard;
	}
	return float4(0,0,0,0);
}