#define PS_RESOLVE
#define SUPPORT_VRR 1
#include "VRX\VRRCommon.hlsl"
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
PUSHCONST cbuffer ResData : register(b2)
{
	int2 Resolution;
};
Texture2D<float4> TempBuffer : register(t0); 
float4 main(VS_OUTPUT input) : SV_Target
{
	int2 Pos = input.uv *Resolution;
	const int2 Rate = int2(2, 2);
	if(IsPixelSource(Pos.xy, Rate))
	{
		discard;
	}
	return TempBuffer[Pos];
}