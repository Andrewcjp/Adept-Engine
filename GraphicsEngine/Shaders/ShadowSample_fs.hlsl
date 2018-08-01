#include "Lighting.hlsl"
#define SHADOW_DEPTH_BIAS 0.005f
#define MAX_LIGHTS 5
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
SamplerState g_sampler : register(s0);
SamplerState g_Clampsampler : register(s1);
TextureCube g_Shadow_texture: register(t0);

float ShadowCalculationCube(const float3 fragPos, Light lpos);

float main(PSInput input) : SV_TARGET
{	
	return ShadowCalculationCube(input.WorldPos.xyz,lights[2]);
}

float ShadowCalculationCube(const float3 fragPos, Light lpos)
{
	// Get vector between fragment position and light position
	float3 fragToLight = (fragPos - lpos.LPosition);
	float currentDepth = length(fragToLight);
	float bias = 0.5f;
	float far_plane = 500;
	float closestDepth = 0;
	closestDepth = g_Shadow_texture.Sample(g_Clampsampler, fragToLight).r;
	closestDepth *= far_plane;
	if (currentDepth - bias > closestDepth)
	{
		return 1.0f;
	}
	return 0.0f;
}
