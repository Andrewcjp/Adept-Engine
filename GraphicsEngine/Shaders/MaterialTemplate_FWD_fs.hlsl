#include "Lighting.hlsl"
SamplerState g_sampler : register(s0);
SamplerState g_Clampsampler : register(s1);
#include "Shadow.hlsl"
//tODO:sub struct for mat data?
cbuffer GOConstantBuffer : register(b0)
{
	row_major matrix Model;
	int HasNormalMap;
	float Roughness;
	float Metallic;
};

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

struct PSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD0;
	float4 WorldPos:TANGENT0;
	row_major float3x3 TBN:TANGENT1;
};

Texture2D g_Shadow_texture[MAX_DIR_SHADOWS]: register(t0, space1);
TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(t1, space2);


TextureCube DiffuseIrMap : register(t10);
TextureCube SpecularBlurMap: register(t11);
Texture2D envBRDFTexture: register(t12);
Texture2D PerSampledShadow: register(t13);

Texture2D g_texture : register(t20);
Texture2D NormalMapTexture : register(t21);

//Declares
float4 main(PSInput input) : SV_TARGET
{
	float2 Pos = input.position.xy / input.position.w;
	////Pos.x = (Pos.x + 1) / 2;
	////Pos.y = (Pos.y + 1) / 2;
	float3 Normal = input.Normal.xyz;
	float out2 = PerSampledShadow.Sample(g_Clampsampler, Pos.xy).r;
	//return float4(out2,0, 0, 1.0f);
#if !TEST
	float3 texturecolour = float3(0, 0, 0);// g_texture.Sample(g_sampler, input.uv).rgb;
#else
	float3 texturecolour = float3(0, 0, 0);
#endif
	//Insert Marker


#if TEST
	texturecolour = Diffuse;
#endif
	
	if (HasNormalMap == 1)
	{
		/*Normal = (NormalMapTexture.Sample(g_sampler, input.uv).xyz)*2.0 - 1.0;
		Normal = normalize(mul(Normal,input.TBN));*/
	}

	float3 irData = DiffuseIrMap.Sample(g_sampler, normalize(Normal)).rgb;
	float3 ViewDir = normalize(CameraPos - input.WorldPos.xyz);
	const float MAX_REFLECTION_LOD = 11.0;
	float3 R = reflect(-ViewDir, Normal);
	float2 envBRDF = envBRDFTexture.Sample(g_sampler,float2(max(dot(Normal, ViewDir), 0.0), Roughness)).rg;
	float3 prefilteredColor = SpecularBlurMap.SampleLevel(g_sampler, R, Roughness * (MAX_REFLECTION_LOD)).rgb;//textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
	float3 output = GetAmbient(normalize(Normal), ViewDir, texturecolour, Roughness, Metallic, irData, prefilteredColor, envBRDF);
	return float4(Normal.xyz, 1.0f);
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		float3 colour = CalcColorFromLight(lights[i], texturecolour, input.WorldPos.xyz,normalize(Normal), CameraPos, Roughness, Metallic);
		if (lights[i].HasShadow && lights[i].type == 0)
		{
			colour *= CalcUnshadowedAmountPCF2x2(lights[i], input.WorldPos, g_Shadow_texture[lights[i].ShadowID]).r;
		}
		if (lights[i].HasShadow && lights[i].type == 1)
		{
			colour *= 1.0 - ShadowCalculationCube(input.WorldPos.xyz, lights[i], g_Shadow_texture2[lights[i].ShadowID]);
		}
		output += colour;
	}
	return float4(output.xyz,1.0f);
}
