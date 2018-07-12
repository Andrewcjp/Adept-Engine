Texture2D PosTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D AlbedoTexture : register(t2);
SamplerState defaultSampler : register (s1);
TextureCube DiffuseIrMap : register(t10);
TextureCube SpecularBlurMap: register(t11);
Texture2D envBRDFTexture: register(t12);
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
	float4 Normalt = NormalTexture.Sample(defaultSampler, input.uv);
	float3 Normal = normalize(Normalt.xyz);
	float4 AlbedoSpec = AlbedoTexture.Sample(defaultSampler, input.uv);
	float Roughness = AlbedoSpec.a;
	float Metallic = Normalt.a;
	//calc Ambient 

	float3 irData = DiffuseIrMap.Sample(defaultSampler, normalize(Normal)).rgb;
	float3 ViewDir = normalize(CameraPos - pos.xyz);
	const float MAX_REFLECTION_LOD = 11.0;
	float3 R = reflect(-ViewDir, Normal);
	float2 envBRDF = envBRDFTexture.Sample(defaultSampler, float2(max(dot(Normal, ViewDir), 0.0), Roughness)).rg;
	float3 prefilteredColor = SpecularBlurMap.SampleLevel(defaultSampler, R, Roughness * (MAX_REFLECTION_LOD)).rgb;
	float3 output = GetAmbient(normalize(Normal), ViewDir, AlbedoSpec.xyz, Roughness, Metallic, irData, prefilteredColor, envBRDF);
	//return float4(irData, 1.0f);
	//return float4(0,0,0, 1.0f);
	for (int i = 0; i < MAX_LIGHT; i++)
	{
		output += CalcColorFromLight(lights[i], AlbedoSpec.xyz, pos.xyz, normalize(Normal.xyz), CameraPos, Roughness, Metallic);
	}
	float gamma = 1.0f / 2.2f;

	//output = output / (output + float3(1.0, 1.0, 1.0));
	//output = pow(output, float3(gamma, gamma, gamma));
	return float4(output,1.0f);
}
