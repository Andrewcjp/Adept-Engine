#include "DefaultShaderCommon.hlsl"
#include "../Lighting.hlsl"
RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);
Texture2D<float4> Normals : register(t5);
Texture2D<float4> Pos : register(t6);
SamplerState g_sampler : register(s0);
SamplerState g_Clampsampler : register(s1);
TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(t5, space2);
#include "../Shadow.hlsl"
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
};
float3 sampleGGXVNDF(float3 Ve, float alpha_x, float alpha_y, float U1, float U2)
{
	// Section 3.2: transforming the view direction to the hemisphere configuration
	float3 Vh = normalize(float3(alpha_x * Ve.x, alpha_y * Ve.y, Ve.z));
	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
	float3 T1 = lensq > 0 ? float3(-Vh.y, Vh.x, 0) * (1.0 / sqrt(lensq)) : float3(1, 0, 0);
	float3 T2 = cross(Vh, T1);
	// Section 4.2: parameterization of the projected area
	float r = sqrt(U1);
	float phi = 2.0 * 3.14 * U2;
	float t1 = r * cos(phi);
	float t2 = r * sin(phi);
	float s = 0.5 * (1.0 + Vh.z);
	t2 = (1.0 - s)*sqrt(1.0 - t1 * t1) + s * t2;
	// Section 4.3: reprojection onto hemisphere
	float3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2))*Vh;
	// Section 3.4: transforming the normal back to the ellipsoid configuration
	float3 Ne = normalize(float3(alpha_x * Nh.x, alpha_y * Nh.y, max(0.0, Nh.z)));
	return Ne;

}
uint rand_xorshift()
{
	uint rng_state = 0;
	// Xorshift algorithm from George Marsaglia's paper
	rng_state ^= (rng_state << 13);
	rng_state ^= (rng_state >> 17);
	rng_state ^= (rng_state << 5);
	return rng_state;
}

StructuredBuffer<Light> LightList : register(t20);
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
#if 1
	ray.Direction = R;
#else
	float f0 = clamp(float(rand_xorshift()) * (1.0 / 4294967296.0), -0.0, 100.0f);
	float f1 = clamp(float(rand_xorshift()) * (1.0 / 4294967296.0), -0.0, 100.0f);
	float2 U = float2(1.0 - f0, 1.0 - f1);
	ray.Direction = sampleGGXVNDF(ViewDir, SmoothNess, SmoothNess, U.x, U.y);
#endif
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
	for (int i = 0; i < LightCount; i++)
	{
		float3 LightColour = CalcColorFromLight(LightList[i], payload.color, payload.Pos, payload.Normal, CameraPos, 0.0f, 0.0f);
		if (LightList[i].HasShadow && LightList[i].type == 1)
		{
			LightColour *= 1.0 - ShadowCalculationCube(payload.Pos.xyz, LightList[i], g_Shadow_texture2[LightList[i].ShadowID]);
		}
		OutColor += LightColour;
	}
	gOutput[launchIndex.xy] = float4(OutColor, SmoothNess);
}
