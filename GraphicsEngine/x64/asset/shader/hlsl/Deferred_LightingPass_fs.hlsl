Texture2D PosTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D AlbedoTexture : register(t2);
SamplerState defaultSampler : register (s1);
#include "Lighting.hlsl"
#define MAX_LIGHT 4

cbuffer LightBuffer : register(b1)
{
	Light lights[MAX_LIGHT];
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	float4 pos = PosTexture.Sample(defaultSampler, input.uv);
	float4 normal = NormalTexture.Sample(defaultSampler, input.uv);
	float4 AlbedoSpec = AlbedoTexture.Sample(defaultSampler, input.uv);
	float3 Diffusecolour = AlbedoSpec.xyz * GetAmbient();
	for (int i = 0; i < MAX_LIGHT; i++)
	{
		Diffusecolour += CalcColorFromLight(lights[i], AlbedoSpec.xyz, pos.xyz, normalize(normal.xyz));
	}
	return float4(Diffusecolour,1.0f);
}
