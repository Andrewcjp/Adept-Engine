#include "DefaultShaderCommon.hlsl"
#define MAX_LIGHTS 1
#include "Lighting.hlsl"
RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);
Texture2D<float4> Normals : register(t5);
Texture2D<float4> Pos : register(t6);
SamplerState g_sampler : register(s0);
SamplerState g_Clampsampler : register(s1);
float3 linearToSrgb(float3 c)
{
	// Based on http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
	float3 sq1 = sqrt(c);
	float3 sq2 = sqrt(sq1);
	float3 sq3 = sqrt(sq2);
	float3 srgb = 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;
	return srgb;
}

cbuffer CameraData: register(b0)
{
	float4x4 viewI;
	float4x4 projectionI;
	float3 CameraPos;
}
cbuffer LightBuffer : register(b1)
{
	int LightCount;
	int4 t;
	//int pad;
	Light lights[MAX_LIGHTS];
};

[shader("raygeneration")]
void rayGen()
{
	
	uint3 launchIndex = DispatchRaysIndex();
	uint3 launchDim = DispatchRaysDimensions();

	float2 crd = float2(launchIndex.xy);
	float2 dims = float2(launchDim.xy);
	float2 NrmPos = crd / dims;
	float2 d = ((crd / dims) * 2.f - 1.f);
	float aspectRatio = dims.x / dims.y;
	const float SmoothNess = Normals.SampleLevel(g_sampler, NrmPos, 0).w;
	
	if (SmoothNess < 0.2)
	{
		gOutput[launchIndex.xy] = float4(0, 0, 0, 0);
		return;
	}
	const float Metalic = Pos.SampleLevel(g_sampler, NrmPos, 0).w;
	RayDesc ray;
	ray.Origin = Pos.SampleLevel(g_sampler, NrmPos, 0).xyz;
	float3 ViewDir = normalize(ray.Origin - CameraPos);
	float3 normal = Normals.SampleLevel(g_sampler, NrmPos, 0).xyz;
	float3 R = reflect(ViewDir, normal);
	//#DXR: Find Importance sampled GCX func?
	ray.Direction = R;

	ray.TMin = 1;
	ray.TMax = 1000;

	RayPayload payload;
	TraceRay(gRtScene, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);
	if (!payload.Hit)
	{
		gOutput[launchIndex.xy] = float4(payload.color, SmoothNess);
		return;
	}
	float3 OutColor = payload.color *GetAmbient_CONST();
	//float3 CalcColorFromLight(Light light, float3 Diffusecolor, float3 FragPos, float3 normal, float3 CamPos, float roughness, float Metalic)
	OutColor += CalcColorFromLight(lights[0], payload.color, payload.Pos, payload.Normal, CameraPos, 0.0f, 0.0f);
	//OutColor = lights[0].color;
	OutColor = payload.color;
	gOutput[launchIndex.xy] = float4(OutColor, SmoothNess);
}
