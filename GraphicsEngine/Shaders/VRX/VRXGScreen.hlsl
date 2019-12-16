#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
RWTexture2D<uint> RateData : register(u0);
PUSHCONST cbuffer Data : register(b1)
{
	int w;
};
PUSHCONST cbuffer Data2 : register(b2)
{
	int h;
};
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const float2 Resolution = float2(w, h);
	const float2 Invres = Resolution * -1;
	float2 ScreenUV = (float2)DTid.xy / (float2)Resolution;
	int ShadingRate = GetShadingRateIDForPixel(ScreenUV, Resolution);
	RateData[DTid.xy/16] = ShadingRate;
}

