#include "Lighting.hlsl"
SamplerState g_sampler : register(s0);
SamplerState g_Clampsampler : register(s1);
#include "Shadow.hlsl"

StructuredBuffer<Light> lights : register(t20);
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

Texture2D GBuffer_Pos:register(t4);
TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(t0, space2);
PUSHCONST cbuffer PreSampleData : register(b3)
{
	int4 Indexes;
};

float Get(float3 pos, int index)
{
	if (index == -1)
	{
		return 0.0f;
	}
	if (length(pos - lights[index].LPosition) > lights[index].Range)
	{
		return 0.0f;
	}
	return ShadowCalculationCube(pos, lights[index], g_Shadow_texture2[lights[index].ShadowID]);
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float3 WorldPos = GBuffer_Pos.Sample(g_sampler,input.uv);
#if MAX_SHADOW_SAMPLES == 1
	return float4(Get(WorldPos, Indexes.x),0,0,0);
#elif MAX_SHADOW_SAMPLES == 2
	return float4(Get(WorldPos, Indexes.x), Get(WorldPos, Indexes.y), 0, 0);
#else
	return float4(Get(WorldPos, Indexes.x), Get(WorldPos, Indexes.y), Get(WorldPos, Indexes.z), Get(WorldPos, Indexes.w));
#endif
}

