#if !FULL_UAV_LOAD
Texture2D<float4> InTex: register(t0);
RWTexture2DArray<uint> DstTexture : register(u0);
#include "Utils/UavLoad.hlsl"

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 groupIndex : SV_GroupID)
{
	float4 ColourSample = InTex[DTid.xy];

	uint test = D3DX_FLOAT2_to_R16G16_FLOAT(float2(ColourSample.r, ColourSample.g));
	float2 RG = D3DX_R16G16_FLOAT_to_FLOAT2(test);

	WriteUAV(ColourSample, DTid.xy, DstTexture);
}
#endif