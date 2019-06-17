#include "DefaultShaderCommon.hlsl"
TextureCube g_texture : register(t10);
SamplerState g_sampler : register(s1);

[shader("miss")]
void Miss(inout RayPayload payload)
{
	float3 sam = g_texture.SampleLevel(g_sampler, WorldRayDirection(), 0).xyz;
	payload.color = sam;	
	payload.Normal = float3(0, 0, 0);
	payload.Hit = false;
	//todo: Use INF here?
	payload.Pos = float3(0, 0, 0);
}