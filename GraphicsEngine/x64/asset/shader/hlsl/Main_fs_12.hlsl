#include "Lighting.hlsl"
//#include "Shadow.hlsl"
#define SHADOW_DEPTH_BIAS 0.005f
#define MAX_SHADOWS 2
cbuffer LightBuffer : register(b1)
{
	Light lights[MAX_LIGHT];
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD0;
	float4 WorldPos:TANGENT0;
};

Texture2D g_texture : register(t0);
Texture2D g_Shadow_texture : register(t1);
TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(t2);
SamplerState g_sampler : register(s0);
SamplerState g_Clampsampler : register(s1);
float GetShadow(float4 pos)
{
	float4 vLightSpacePos = pos;
	//vLightSpacePos = mul(vLightSpacePos, lights[lightIndex].view);
	//vLightSpacePos = mul(vLightSpacePos, lights[lightIndex].projection);
	vLightSpacePos = mul(vLightSpacePos, lights[0].LightVP);
	vLightSpacePos.xyz /= vLightSpacePos.w;
	float2 vShadowTexCoord = 0.5f * vLightSpacePos.xy + 0.5f;
	vShadowTexCoord.y = 1.0f - vShadowTexCoord.y;
	float bias = 0.005;
	if (g_Shadow_texture.Sample(g_sampler, vShadowTexCoord.xy).r < (vLightSpacePos.z - bias))
	{
		return 1.0f;
	}
	return 0.0f;
}
float4 CalcUnshadowedAmountPCF2x2(int lightid, float4 vPosWorld)
{
	// Compute pixel position in light space.
	float4 vLightSpacePos = vPosWorld;
	vLightSpacePos = mul(vLightSpacePos, lights[lightid].LightVP);

	vLightSpacePos.xyz /= vLightSpacePos.w;

	// Translate from homogeneous coords to texture coords.
	float2 vShadowTexCoord = 0.5f * vLightSpacePos.xy + 0.5f;
	vShadowTexCoord.y = 1.0f - vShadowTexCoord.y;

	// Depth bias to avoid pixel self-shadowing.
	float vLightSpaceDepth = vLightSpacePos.z - SHADOW_DEPTH_BIAS;

	// Find sub-pixel weights.//todo: shader define!
	float2 vShadowMapDims = float2(1024, 1024); // need to keep in sync with .cpp file
	float size = 1.0f;
	float4 vSubPixelCoords = float4(size, size, size, size);
	vSubPixelCoords.xy = frac(vShadowMapDims * vShadowTexCoord);
	vSubPixelCoords.zw = 1.0f - vSubPixelCoords.xy;
	float4 vBilinearWeights = vSubPixelCoords.zxzx * vSubPixelCoords.wwyy;

	// 2x2 percentage closer filtering.
	float2 vTexelUnits = 1.0f / vShadowMapDims;
	float4 vShadowDepths;
	if (lightid == 0)
	{
		vShadowDepths.x = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord);
		vShadowDepths.y = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord + float2(vTexelUnits.x, 0.0f));
		vShadowDepths.z = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord + float2(0.0f, vTexelUnits.y));
		vShadowDepths.w = g_Shadow_texture.Sample(g_Clampsampler, vShadowTexCoord + vTexelUnits);
	}
	else
	{
		/*vShadowDepths.x = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord);
		vShadowDepths.y = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord + float2(vTexelUnits.x, 0.0f));
		vShadowDepths.z = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord + float2(0.0f, vTexelUnits.y));
		vShadowDepths.w = g_Shadow_texture2.Sample(g_Clampsampler, vShadowTexCoord + vTexelUnits);*/
	}
	// What weighted fraction of the 4 samples are nearer to the light than this pixel?
	float4 vShadowTests = (vShadowDepths >= vLightSpaceDepth) ? 1.0f : 0.0f;
	return dot(vBilinearWeights, vShadowTests);
}
float ShadowCalculationCube(const float3 fragPos, Light lpos);

float4 main(PSInput input) : SV_TARGET
{

	float3 texturecolour = g_texture.Sample(g_sampler, input.uv);
	float3 output = float3(0, 0, 0);
	for (int i = 0; i < MAX_LIGHT; i++)
	{
		float3 colour = CalcColorFromLight(lights[i], texturecolour, input.WorldPos.xyz, normalize(input.Normal.xyz));
		if (lights[i].HasShadow && lights[i].type == 0)
		{
			colour *= CalcUnshadowedAmountPCF2x2(i, input.WorldPos);
		}
		if (lights[i].HasShadow && lights[i].type == 1)
		{
			colour *= 1.0 - ShadowCalculationCube(input.WorldPos.xyz, lights[i]);
		}
		output += colour;
	}

	float3 ambeint = texturecolour * GetAmbient();
	float Shadow = CalcUnshadowedAmountPCF2x2(0, input.WorldPos);
	float3 GammaCorrected = ambeint + output;
	float gamma = 1.0f / 2.2f;
	//	GammaCorrected = pow(GammaCorrected, float4(gamma, gamma, gamma, gamma));
	return float4(GammaCorrected.xyz,1.0f);
}


float ShadowCalculationCube(const float3 fragPos, Light lpos)
{
	// Get vector between fragment position and light position
	float3 fragToLight = (fragPos - lpos.LPosition);
	float currentDepth = length(fragToLight);
	//if (currentDepth > MaxShadowDistance)
	//{
	//	//	return 0.0f;
	//}
	float shadow = 0.0f;
	float bias = 0.5f;
	int samples = 1;
	/*float viewDistance = length(viewPos - fragPos);
	float diskRadius = (1.0f + (viewDistance / far_plane)) / 25.0f;*/
	float far_plane = 500;

	float closestDepth = 0;
	float3 val = float3(1, 0, 0);
	/*if (lpos.ShadowID == 0)
	{
		return 0.0f;
	}*/
	int id = lpos.ShadowID;
	//if (lpos.ShadowID == 0)
	//{
	//	id = 1;
	//}
	//if (lpos.ShadowID == 1)
	//{
	//	id = 0;
	//}
	closestDepth = g_Shadow_texture2[id].Sample(g_Clampsampler, (fragToLight)).r;

	closestDepth *= far_plane;

	if (currentDepth - bias > closestDepth)
	{
		return 1.0f;
	}

	//shadow /= float(samples);//average of samples
	//						 //cleanup the low shadow areas
	/*if (shadow < 0.25f)
	{
		return 0.0f;
	}*/
	return 0.0f;
}
