#include "DefaultShaderCommon.hlsl"
struct VertexData
{
	float3 pos;
	float3 Normal;
	float2 UV;
	float pad[11];
};
StructuredBuffer<min16uint> indices			: register(t1);
StructuredBuffer<VertexData> vertices		: register(t2);
Texture2D<float4> albedo					: register(t3);
SamplerState g_sampler : register(s0);

//#include "RaytracingCommon.hlsl"

[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs : SV_IntersectionAttributes)
{
	uint instanceID = InstanceID();
	float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
	//we have the triangle index 
	int IndexStart = PrimitiveIndex() * 3;
	///find the tri in the indexes
	int3 indcis = int3(indices[IndexStart], indices[IndexStart + 1], indices[IndexStart + 2]);
	//build the interpolant at this pixel from the 3 vertexes that make up this tri
	//the barycentrics defines the pos on the triangle so the weight of each vertex.
	float2 UVAtPixel = float2(0, 0);
	for (int i = 0; i < 3; i++)
	{
		VertexData vertex = vertices[indcis[i]];
		UVAtPixel += vertex.UV * barycentrics[i];
	}
	payload.color = albedo.SampleLevel(g_sampler, UVAtPixel, 5).xyz;
	//payload.color = float3(Value, 0, 0);
	//payload.color = vertex.Normal;
//	payload.color = float3(UVAtPixel, 0);
}
