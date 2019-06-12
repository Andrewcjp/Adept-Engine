#include "DefaultShaderCommon.hlsl"
RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);
Texture2D<float4> Normals : register(t5);
Texture2D<float4> Pos : register(t6);
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
}

[shader("raygeneration")]
void rayGen()
{
	uint3 launchIndex = DispatchRaysIndex();
	uint3 launchDim = DispatchRaysDimensions();

	float2 crd = float2(launchIndex.xy);
	float2 dims = float2(launchDim.xy);

	float2 d = ((crd / dims) * 2.f - 1.f);
	float aspectRatio = dims.x / dims.y;
	const float SmoothNess = Normals[launchIndex.xy].w;
	if (SmoothNess < 0.2)
	{
		return;
	}
#if 0
	int Samples = 2;
	int HalfSamples = Samples / 2;
	float OffsetScale = 0.01f;
	float3 AccumColour = float3(0, 0, 0);
	for (int i = -HalfSamples; i < HalfSamples; i++)
	{
		for (int y = -HalfSamples; y < HalfSamples; y++)
		{
			RayDesc ray;
			ray.Direction = Normals[launchIndex.xy].xyz;
			ray.Origin = Pos[launchIndex.xy];
			ray.Origin += float3(OffsetScale * i, OffsetScale*y, 0);
			ray.TMin = 1;
			ray.TMax = 1000;

			RayPayload payload;
			TraceRay(gRtScene, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);
			AccumColour += payload.color;
		}
	}
	AccumColour /= Samples * Samples;
	gOutput[launchIndex.xy] = lerp(float4(AccumColour, 1), gOutput[launchIndex.xy], 1.0 - SmoothNess);
#else
	RayDesc ray;
	ray.Direction = Normals[launchIndex.xy].xyz;
	ray.Origin = Pos[launchIndex.xy];
	ray.TMin = 1;
	ray.TMax = 1000;

	RayPayload payload;
	TraceRay(gRtScene, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);
	gOutput[launchIndex.xy] = lerp(float4(payload.color, 1), gOutput[launchIndex.xy], 1.0 - SmoothNess);
#endif
}
