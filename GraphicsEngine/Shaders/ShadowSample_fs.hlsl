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

TextureCube g_Shadow_texture: register(t0);

cbuffer PreSampleData : register(b3)
{
	int index;
};

float4 main(PSInput input) : SV_TARGET
{
#if MAX_SHADOW_SAMPLES == 1
	return float4(ShadowCalculationCube(input.WorldPos.xyz,lights[index],g_Shadow_texture),0,0,0);
#else
	return float4(ShadowCalculationCube(input.WorldPos.xyz, lights[index], g_Shadow_texture), 0, 0, 0);
#endif
}

