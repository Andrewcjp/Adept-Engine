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
#if 1//WITHRT
Texture2D ScreenSpaceSpec: register(t14);
#endif
#if !VULKAN
Texture2D g_Shadow_texture[MAX_DIR_SHADOWS]: register(t4, space1);
TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(t5, space2);
#endif

#include "Lighting.hlsl"
#include "Shadow.hlsl"
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
};
#define VRS_TILE_SIZE 16
#define SUPPORT_VRR 1
#define SHADER_SUPPORT_VRR 1
#include "VRX\VRRCommon.hlsl"
#include "ReflectionEnviroment.hlsl"
float3 GetSpecular(float2 ScreenPos, float3 R, float Roughness)
{
#if 1//WITHRT
	float4 prefilteredColor = ScreenSpaceSpec.Sample(g_Clampsampler, ScreenPos.xy);
	if (prefilteredColor.a > 0.0)
	{
		return prefilteredColor.xyz;
	}
#endif

	return GetReflectionColor(R, Roughness);
}
#define VOXEL_TRACE 0
#if VOXEL_TRACE
#include "Shading/ReflectionsTrace.hlsl"
#include "Voxel/VoxelTrace_PS.hlsl"
Texture3D<uint4> voxelTex :  register(t50);
#endif
#define SHOW_SHADOW 0
float4 main(VS_OUTPUT input) : SV_Target
{
#if 0// SHADER_SUPPORT_VRR 
	if (!ShouldShadePixel(input.uv,Resolution))
	{
		//discard;
		return float4(0, 0, 0, 0);
	}
#endif

	float4 pos = PosTexture.Sample(defaultSampler, input.uv);
	float4 Normalt = NormalTexture.Sample(defaultSampler, input.uv);
	float3 Normal = normalize(Normalt.xyz);
	float4 AlbedoSpec = AlbedoTexture.Sample(defaultSampler, input.uv);
	float Roughness = AlbedoSpec.a;
	float Metallic = Normalt.a;
	float3 ViewDir = normalize(CameraPos - pos.xyz);
#if VOXEL_TRACE
	float4 voxelSpec = float4(0, 0, 0,0);
	if (Roughness > 0.8 /*&& Roughness < 0.8*/)
	{
		float aperture = tan(Roughness * 3.14 * 0.5f * 0.1f);
		uint2 pixelindex = input.uv * Resolution;
		uint seed = initRand(pixelindex.x + pixelindex.y*Resolution.x, 0);
		float3 H = getGGXMicrofacet(seed, 1.0 - Roughness, Normal);
		float3 V = ViewDir;
		//float3 coneDirection = normalize(2.f * dot(V, H) * H - V);
		float3 coneDirection = reflect(-ViewDir, Normal);
		voxelSpec = ConeTrace_FixedMip(voxelTex, pos.xyz, Normal, coneDirection, aperture);
		//exec lighting
		return voxelSpec;
	}
#endif
	float3 irData = DiffuseIrMap.Sample(defaultSampler, normalize(Normal)).rgb;


	float3 R = reflect(-ViewDir, Normal);
	float2 envBRDF = envBRDFTexture.Sample(defaultSampler, float2(max(dot(Normal, ViewDir), 0.0), Roughness)).rg;
	float3 prefilteredColor = GetSpecular(input.uv,R, Roughness);
#if VOXEL_TRACE
	if (voxelSpec.a > 0.0f)
	{
		prefilteredColor = voxelSpec;
	}
#endif
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
