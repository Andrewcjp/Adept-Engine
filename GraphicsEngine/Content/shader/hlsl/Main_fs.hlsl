#include "Lighting.hlsl"
//#include "Shadow.hlsl"
#define SHADOW_DEPTH_BIAS 0.005f
cbuffer LightBuffer : register(b1)
{
	Light lights[MAX_LIGHTS];
};
//tODO:sub struct for mat data?
cbuffer GOConstantBuffer : register(b0)
{
	row_major matrix Model;
	int HasNormalMap;
	float Roughness;
	float Metallic;
};

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
SamplerState g_sampler : register(s0);
SamplerState g_Clampsampler : register(s1);
Texture2D g_texture : register(t0);
Texture2D NormalMapTexture : register(t1);


Texture2D g_Shadow_texture[MAX_DIR_SHADOWS]: register(t3);
TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(POINT_SHADOW_OFFSET);
TextureCube DiffuseIrMap : register(t10);
TextureCube SpecularBlurMap: register(t11);
Texture2D envBRDFTexture: register(t12);
Texture2D PerSampledShadow: register(t13);
float GetShadow(float4 pos)
{
	float4 vLightSpacePos = pos;
	//vLightSpacePos = mul(vLightSpacePos, lights[lightIndex].view);
	//vLightSpacePos = mul(vLightSpacePos, lights[lightIndex].projection);
	vLightSpacePos = mul(vLightSpacePos, lights[0].LightVP);
	vLightSpacePos.xyz /= vLightSpacePos.w;
	float2 vShadowTexCoord = 0.5f * vLightSpacePos.xy + 0.5f;
	vShadowTexCoord.y = 1.0f - vShadowTexCoord.y;
	float bias = 0.005f;
	if (g_Shadow_texture[0].Sample(g_sampler, vShadowTexCoord.xy).r < (vLightSpacePos.z - bias))
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
	float4 vShadowDepths = float4(0,0,0,0);
	int id = 0;
	if (lightid == 0)
	{
		vShadowDepths.x = g_Shadow_texture[id].Sample(g_Clampsampler, vShadowTexCoord).r;
		vShadowDepths.y = g_Shadow_texture[id].Sample(g_Clampsampler, vShadowTexCoord + float2(vTexelUnits.x, 0.0f)).r;
		vShadowDepths.z = g_Shadow_texture[id].Sample(g_Clampsampler, vShadowTexCoord + float2(0.0f, vTexelUnits.y)).r;
		vShadowDepths.w = g_Shadow_texture[id].Sample(g_Clampsampler, vShadowTexCoord + vTexelUnits).r;
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
	float2 Pos = input.position.xy / input.position.w;
	////Pos.x = (Pos.x + 1) / 2;
	////Pos.y = (Pos.y + 1) / 2;
	float out2 = PerSampledShadow.Sample(g_Clampsampler, Pos.xy).r;
	//return float4(out2,0, 0, 1.0f);
	float3 texturecolour = g_texture.Sample(g_sampler, input.uv).rgb;
	//Insert Marker

	float3 Normal = input.Normal.xyz;
	if (HasNormalMap == 1)
	{
		Normal = (NormalMapTexture.Sample(g_sampler, input.uv).xyz)*2.0 - 1.0;
		Normal = normalize(mul(Normal,input.TBN));
	}

	float3 irData = DiffuseIrMap.Sample(g_sampler, normalize(Normal)).rgb;
	float3 ViewDir = normalize( CameraPos- input.WorldPos.xyz);
	const float MAX_REFLECTION_LOD = 11.0;
	float3 R = reflect(-ViewDir, Normal);
	float2 envBRDF = envBRDFTexture.Sample(g_sampler,float2(max(dot(Normal, ViewDir), 0.0), Roughness)).rg;
	float3 prefilteredColor = SpecularBlurMap.SampleLevel(g_sampler, R, Roughness * (MAX_REFLECTION_LOD)).rgb;//textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
	float3 output = GetAmbient(normalize(Normal), ViewDir, texturecolour, Roughness, Metallic, irData, prefilteredColor, envBRDF);

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		float3 colour = CalcColorFromLight(lights[i], texturecolour, input.WorldPos.xyz,normalize(Normal), CameraPos, Roughness, Metallic);
		if (lights[i].HasShadow && lights[i].type == 0)
		{
			colour *= CalcUnshadowedAmountPCF2x2(i, input.WorldPos).r;
		}
		if (i == 2)
		{
			float out2 = PerSampledShadow.Sample(g_Clampsampler, input.uv).r;
			colour *= 1.0 - out2;
		}
		else
		{
			if (lights[i].HasShadow && lights[i].type == 1)
			{
				colour *= 1.0 - ShadowCalculationCube(input.WorldPos.xyz, lights[i]);
			}
		}
	
		output += colour;
	}
	return float4(output.xyz,1.0f);
}


float ShadowCalculationCube(const float3 fragPos, Light lpos)
{
	// Get vector between fragment position and light position
	float3 fragToLight = (fragPos - lpos.LPosition);
	float currentDepth = length(fragToLight);
	float bias = 0.5f;
	float far_plane = 500;
	float closestDepth = 0;
	int id = lpos.ShadowID;
	closestDepth = g_Shadow_texture2[id].Sample(g_Clampsampler, fragToLight).r;
	closestDepth *= far_plane;
	if (currentDepth - bias > closestDepth)
	{
		return 1.0f;
	}
	return 0.0f;
}
