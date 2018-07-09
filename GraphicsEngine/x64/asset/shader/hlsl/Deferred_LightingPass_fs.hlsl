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
cbuffer SceneConstantBuffer : register(b2)
{
	row_major matrix View;
	row_major matrix Projection;
	float3 CameraPos;
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
	float Roughness = AlbedoSpec.a;
	float Metallic = normal.a;
	float3 Diffusecolour = AlbedoSpec.xyz * GetAmbient_CONST();
	for (int i = 0; i < MAX_LIGHT; i++)
	{
		Diffusecolour += CalcColorFromLight(lights[i], AlbedoSpec.xyz, pos.xyz, normalize(normal.xyz), CameraPos, Roughness, Metallic);
	}
	float gamma = 1.0f / 2.2f;

	Diffusecolour = Diffusecolour / (Diffusecolour + float3(1.0, 1.0, 1.0));
	Diffusecolour = pow(Diffusecolour, float3(gamma, gamma, gamma));
	return float4(Diffusecolour,1.0f);
}
