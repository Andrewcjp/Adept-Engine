#include "Lighting.hlsl"
SamplerState g_sampler : register(s0);
SamplerState g_Clampsampler : register(s1);
#include "Shadow.hlsl"

#define MAX_LIGHTS 8
cbuffer LightBuffer : register(b1)
{
	Light lights[MAX_LIGHTS];
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float4 WorldPos:TANGENT0;
};

TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(t0, space2);
cbuffer PreSampleData : register(b3)
{
	int4 Indexes;
};

float Get(float3 pos, int index)
{
	return ShadowCalculationCube(pos, lights[index], g_Shadow_texture2[lights[index].ShadowID]);
}

float4 main(PSInput input) : SV_TARGET
{
#if MAX_SHADOW_SAMPLES == 1
	return float4(Get(input.WorldPos.xyz, Indexes.x),0,0,0);
#elif MAX_SHADOW_SAMPLES == 2
	return float4(Get(input.WorldPos.xyz, Indexes.x), Get(input.WorldPos.xyz, Indexes.y), 0, 0);
#else
	return float4(Get(input.WorldPos.xyz, Indexes.x), Get(input.WorldPos.xyz, Indexes.y), Get(input.WorldPos.xyz, Indexes.z), Get(input.WorldPos.xyz, Indexes.w));
#endif
}

