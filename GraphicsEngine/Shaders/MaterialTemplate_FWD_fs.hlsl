#include "Lighting.hlsl"
#include "Core/Common.hlsl"
SamplerState defaultSampler : register(s0);
SamplerState g_Clampsampler : register(s1);
#include "Shadow.hlsl"

//tODO:sub struct for mat data?
cbuffer GOConstantBuffer : register(b0)
{
	row_major matrix Model;
};
//BufferPoint

cbuffer LightBuffer : register(b1)
{
	int LightCount;
#if 1//VULKAN
	uint2 TileCount;
#else
	uint2 TileCount;
	int pad;
#endif

};

StructuredBuffer<Light> lights : register(t30);
StructuredBuffer<int> LightIndexs : register(t31);

cbuffer SceneConstantBuffer : register(b2)
{
	row_major matrix View;
	row_major matrix Projection;
	float3 CameraPos;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD0;
	float4 WorldPos:TANGENT0;
	row_major float3x3 TBN:TANGENT1;
};
#ifdef WITH_SHADOW
Texture2D g_Shadow_texture[MAX_DIR_SHADOWS]: register(t0, space1);
TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(t1, space2);
#endif
#define MAX_CUBEMAPS 1
TextureCube DiffuseIrMap : register(t10);
#if VULKAN
TextureCube SpecularBlurMap[MAX_CUBEMAPS]: register(t11/*,space3*/);
#else
TextureCube SpecularBlurMap[MAX_CUBEMAPS]: register(t11, space3);
#endif
Texture2D envBRDFTexture: register(t12);
//PreSampled
Texture2D PerSampledShadow: register(t13);
#if !VULKAN
cbuffer Resolution : register(b5)
{
	int2 Res;
};
#endif
#include "ReflectionEnviroment.hlsl"

float FWD_GetPresampledShadow(float2 pos, int index)
{
	float vis = (1.0 - PerSampledShadow.Sample(g_Clampsampler, pos.xy)[index]);
	return vis;
}
//Declares
float4 main(PSInput input) : SV_TARGET
{
#if VULKAN
	int2 Res = int2(1,1);
#endif
	const float2 ScreenPos = input.position.xy / Res; //Compute Position  for this pixel in 0-1 space
	float3 Normal = input.Normal.xyz;
#if !TEST
	float3 texturecolour = float3(0, 0, 0);
#else
	float3 texturecolour = float3(0, 0, 0);
#endif
	//Insert Marker


#if TEST
	texturecolour = Diffuse;
#endif
//#if VULKAN
//	return float4(texturecolour, 1.0f);
//#endif
	float3 irData = DiffuseIrMap.Sample(defaultSampler, normalize(Normal)).rgb;
	float3 ViewDir = normalize(CameraPos - input.WorldPos.xyz);

	float3 R = reflect(-ViewDir, Normal);
	float2 envBRDF = envBRDFTexture.Sample(defaultSampler,float2(max(dot(Normal, ViewDir), 0.0), Roughness)).rg;
	//float3 prefilteredColor = SpecularBlurMap.SampleLevel(defaultSampler, R, Roughness * (MAX_REFLECTION_LOD)).rgb;
	float3 prefilteredColor = GetReflectionColor(R, Roughness);
	float3 output = GetAmbient(normalize(Normal), ViewDir, texturecolour, Roughness, Metallic, irData, prefilteredColor, envBRDF);

	float2 pixel = input.position.xy;
	uint2 tileIndex = uint2(floor(pixel / LIGHTCULLING_TILE_SIZE));
	uint startOffset = flatten2D(tileIndex, TileCount.xy) * MAX_LIGHTS;

	uint count = LightIndexs[startOffset];
	startOffset += 1;

	[unroll(MAX_LIGHTS)]
	for (int i = 0; i < count; i++)
	{
#ifdef LIGHT_CULLING
		int index = LightIndexs[startOffset + i];
#else
		int index = i;
#endif
		float3 colour = CalcColorFromLight(lights[index], texturecolour, input.WorldPos.xyz,normalize(Normal), CameraPos, Roughness, Metallic);
#ifdef WITH_SHADOW
		[branch] if (lights[index].HasShadow && lights[index].PreSampled.x)
		{
			colour *= FWD_GetPresampledShadow(ScreenPos,lights[index].PreSampled.y);
		}
		else if (lights[index].HasShadow && lights[index].type == 0)
		{
			colour *= CalcUnshadowedAmountPCF2x2(lights[index], input.WorldPos, g_Shadow_texture[lights[index].ShadowID]).r;
		}
		else if (lights[index].HasShadow && lights[index].type == 1)
		{
			colour *= 1.0 - ShadowCalculationCube(input.WorldPos.xyz, lights[index], g_Shadow_texture2[lights[index].ShadowID]);
		}
#endif
		output += colour;
	}
	return float4(output.xyz,0.5f);
}
