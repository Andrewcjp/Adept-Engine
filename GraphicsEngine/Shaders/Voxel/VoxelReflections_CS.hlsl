#include "Shading/ReflectionsTrace.hlsl"
#include "../Lighting.hlsl"
Texture3D<uint4> voxelTex :  register(t50);
RWTexture2D<float4> gOutput : register(u0);
Texture2D<float4> GBUFFER_Normal : register(t5);
Texture2D<float4> GBUFFER_Pos : register(t6);
Texture2D<float4> GBUFFER_BaseSpec : register(t7);
TextureCube SpecularBlurMap: register(t11);
SamplerState Textures : register (s0);
TextureCube ShadowData[MAX_POINT_SHADOWS] : register(t12, space2);
//SamplerState defaultSampler : register(s1);
SamplerState g_Clampsampler : register(s1);
#include "../Shadow.hlsl"
#include "Voxel/VoxelTrace_PS.hlsl"
cbuffer ConstData: register(b0)
{
	float3 CameraPos;
	int2 Res;
	int FrameCount;
}
cbuffer RTBufferData : register(b2)
{
	float RT_RoughnessThreshold;
	float VX_MaxRoughness;
	float VX_MinRoughness;
	float VX_RT_BlendStart;
	float VX_RT_BlendEnd;
	float VX_RT_BlendFactor;
	int Max_VXRayCount;
	int Max_RTRayCount;
};
cbuffer LightBuffer : register(b1)
{
	int LightCount;
};
StructuredBuffer<Light> LightList : register(t20);
float3 LaunchSample(float3 origin, uint seed, float SmoothNess, float3 normal, float3 V)
{
	float3 H = getGGXMicrofacet(seed, 1.0 - SmoothNess, normal);
	float3 coneDirection = normalize(2.f * dot(V, H) * H - V);
	//coneDirection = reflect(-V, normal);
	TracePayload payload = ConeTrace_FixedMip_Loop2(voxelTex, origin, normalize(normal), coneDirection);
	if (payload.alpha == 0.0f)
	{
		return SpecularBlurMap.SampleLevel(Textures, payload.Normal, SmoothNess * (MAX_REFLECTION_LOD)).rgb;
	}
	float3 OutColor = payload.color.xyz *GetAmbient_CONST();
	//float3 CalcColorFromLight(Light light, float3 Diffusecolor, float3 FragPos, float3 normal, float3 CamPos, float roughness, float Metalic)
	for (int i = 0; i < LightCount; i++)
	{
		float3 LightColour = CalcColorFromLight(LightList[i], payload.color, payload.pos, payload.Normal, CameraPos, 0.0f, 0.0f);
		if (LightList[i].HasShadow && LightList[i].type == 1)
		{
			LightColour *= 1.0 - ShadowCalculationCube(payload.pos.xyz, LightList[i], ShadowData[LightList[i].ShadowID]);
		}
		OutColor += LightColour;
	}
	return OutColor;
}
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float2 crd = float2(DTid.xy);
	float2 dims = float2(Res.xy);
	float2 NrmPos = crd / dims;
	const float Roughness = GBUFFER_BaseSpec.SampleLevel(g_Clampsampler, NrmPos, 0).w;
	if (Roughness >= VX_MinRoughness && Roughness < VX_MaxRoughness)
	{
		float3 Pos = GBUFFER_Pos.SampleLevel(g_Clampsampler, NrmPos, 0).xyz;
		float3 Normal = GBUFFER_Normal.SampleLevel(g_Clampsampler, NrmPos, 0).xyz;
		float3 ViewDir = normalize(CameraPos-Pos);
		int SampleCount = Max_VXRayCount;
		if (Roughness > 0.9)
		{
			SampleCount = 1;
		}
		float3 AccumColour = float3(0, 0, 0);
		for (int i = 0; i < SampleCount; i++)
		{
			uint seed = initRand(DTid.x + DTid.y*Res.x, FrameCount+i);
			AccumColour += LaunchSample(Pos, seed, Roughness, Normal, ViewDir);
		}
		AccumColour /= SampleCount;
		gOutput[DTid.xy] = float4(AccumColour, Roughness);
	}
	else
	{
		gOutput[DTid.xy] = float4(0, 0, 0, 0);
	}
}