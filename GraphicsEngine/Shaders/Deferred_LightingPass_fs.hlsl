SamplerState defaultSampler : register (s0);
SamplerState g_Clampsampler : register(s1);

Texture2D PosTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D AlbedoTexture : register(t2);

TextureCube DiffuseIrMap : register(t10);
TextureCube SpecularBlurMap: register(t11);
Texture2D envBRDFTexture: register(t12);

Texture2D PerSampledShadow: register(t13);


Texture2D g_Shadow_texture[MAX_DIR_SHADOWS]: register(t4, space1);
TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(t5, space2);


#include "Lighting.hlsl"
#include "Shadow.hlsl"


cbuffer LightBuffer : register(b1)
{
	Light lights[MAX_LIGHTS];
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
#define SHOW_SHADOW 0
float4 main(VS_OUTPUT input) : SV_Target
{
	//return float4(PerSampledShadow.Sample(g_Clampsampler, input.uv).r,0.0,0.0,1.0f);

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
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		float3 LightColour = CalcColorFromLight(lights[i], AlbedoSpec.xyz, pos.xyz, normalize(Normal.xyz), CameraPos, Roughness, Metallic);
		if (lights[i].PreSampled.x)
		{
#if SHOW_SHADOW
			const float vis = (1.0 - PerSampledShadow.Sample(g_Clampsampler, input.uv)[lights[i].PreSampled.y]);
			if (vis == 0.0f)
			{
				LightColour = float3(0,1,0);
			}
			else
			{
				LightColour *= vis;
			}
#else
			LightColour *= (1.0 - PerSampledShadow.Sample(g_Clampsampler, input.uv)[lights[i].PreSampled.y]);
#endif
		}
		else
		{
			if (lights[i].HasShadow && lights[i].type == 1)
			{
				LightColour *= 1.0 - ShadowCalculationCube(pos.xyz, lights[i], g_Shadow_texture2[lights[i].ShadowID]);
			}
		}
		output += LightColour;
	}
	return float4(output,1.0f);
}
