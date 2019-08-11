SamplerState defaultSampler : register (s0);
SamplerState g_Clampsampler : register(s1);

Texture2D PosTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D AlbedoTexture : register(t2);
#define MAX_CUBEMAPS 1
TextureCube DiffuseIrMap : register(t10);
TextureCube SpecularBlurMap[MAX_CUBEMAPS]: register(t11);

Texture2D envBRDFTexture: register(t12);

Texture2D PerSampledShadow: register(t13);
#if WITHRT
Texture2D ScreenSpaceSpec: register(t14);
#endif
#if !VULKAN
Texture2D g_Shadow_texture[MAX_DIR_SHADOWS]: register(t4, space1);
TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(t5, space2);
#endif

#include "Lighting.hlsl"
#include "Shadow.hlsl"
#include "VRX\VRRCommon.hlsl"
#include "VRX\VRSCommon.hlsl"
cbuffer LightBuffer : register(b1)
{
	int LightCount;
	int4 t;
};

StructuredBuffer<Light> LightList : register(t20);
StructuredBuffer<int> LightIndexs : register(t21);
cbuffer SceneConstantBuffer : register(b2)
{
	row_major matrix View;
	row_major matrix Projection;
	float3 CameraPos;
	int2 Resolution;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	VRR_SHADER_PAYLOAD
};

#include "ReflectionEnviroment.hlsl"
float3 GetSpecular(float2 ScreenPos, float3 R, float Roughness)
{
#if WITHRT
	float4 prefilteredColor = ScreenSpaceSpec.Sample(g_Clampsampler, ScreenPos);
	if (prefilteredColor.a > 0.0)
	{
		return prefilteredColor.xyz;
	}
#endif
	return GetReflectionColor(R, Roughness);
}
#define SHOW_SHADOW 0
float4 main(VS_OUTPUT input) : SV_Target
{
#if SUPPORT_VRS
	if (!ShouldShadePixel(input.uv,Resolution))
	{
		return float4(1, 1, 1, 0);
	}	
#endif

	float4 pos = PosTexture.Sample(defaultSampler, input.uv);
	float4 Normalt = NormalTexture.Sample(defaultSampler, input.uv);
	float3 Normal = normalize(Normalt.xyz);
	float4 AlbedoSpec = AlbedoTexture.Sample(defaultSampler, input.uv);
	float Roughness = AlbedoSpec.a;
	float Metallic = Normalt.a;

	float3 irData = DiffuseIrMap.Sample(defaultSampler, normalize(Normal)).rgb;
	float3 ViewDir = normalize(CameraPos - pos.xyz);

	float3 R = reflect(-ViewDir, Normal);
	float2 envBRDF = envBRDFTexture.Sample(defaultSampler, float2(max(dot(Normal, ViewDir), 0.0), Roughness)).rg;
	float3 prefilteredColor = GetSpecular(input.uv,R, Roughness);
	float3 output = GetAmbient(normalize(Normal), ViewDir, AlbedoSpec.xyz, Roughness, Metallic, irData, prefilteredColor, envBRDF);
	[unroll(MAX_LIGHTS)]
	for (int i = 0; i < LightCount; i++)
	{
		float3 LightColour = CalcColorFromLight(LightList[i], AlbedoSpec.xyz, pos.xyz, normalize(Normal.xyz), CameraPos, Roughness, Metallic);
#if 1
#if !VULKAN
		if (LightList[i].PreSampled.x)
		{
#if SHOW_SHADOW
			const float vis = (1.0 - PerSampledShadow.Sample(g_Clampsampler, input.uv)[LightList[i].PreSampled.y]);
			if (vis == 0.0f)
			{
				LightColour = float3(0,1,0);
			}
			else
			{
				LightColour *= vis;
			}
#else
			LightColour *= (1.0 - PerSampledShadow.Sample(g_Clampsampler, input.uv)[LightList[i].PreSampled.y]);
#endif
		}
		else
		{
			if (LightList[i].HasShadow && LightList[i].type == 1)
			{
				LightColour *= 1.0 - ShadowCalculationCube(pos.xyz, LightList[i], g_Shadow_texture2[LightList[i].ShadowID]);
			}
		}
#endif
#endif
		output += LightColour;
	}
	return float4(output,1.0f);
}
