#include "DefaultShaderCommon.hlsl"
#include "RaytracingCommon.hlsl"
Texture2D<float4> albedo					: register(t3);
SamplerState g_sampler : register(s0);



[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs : SV_IntersectionAttributes)
{
	uint instanceID = InstanceID();
	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	//we have the triangle index 

	VertexAttributes vert = GetVertexData(PrimitiveIndex(), barycentrics);

	payload.color = albedo.SampleLevel(g_sampler, vert.uv, 0).xyz;

}
