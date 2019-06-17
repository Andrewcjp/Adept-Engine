struct VertexAttributes
{
	float3 position;
	float2 uv;
	float3 Normal;
};
struct VertexData
{
	float3 pos;
	float3 Normal;
	float2 UV;
	float pad[11];
};
StructuredBuffer<min16uint> indices			: register(t1);
StructuredBuffer<VertexData> vertices		: register(t2);

VertexAttributes GetVertexData(int primitiveIndex, float3 Bary)
{
	int IndexStart = primitiveIndex * 3;
	///find the tri in the indexes
	int3 indcis = int3(indices[IndexStart], indices[IndexStart + 1], indices[IndexStart + 2]);
	//build the interpolant at this pixel from the 3 vertexes that make up this tri
	//the barycentrics defines the pos on the triangle so the weight of each vertex.
	VertexAttributes Vert = (VertexAttributes)0;
	for (int i = 0; i < 3; i++)
	{
		VertexData vertex = vertices[indcis[i]];
		Vert.uv += vertex.UV * Bary[i];
		Vert.Normal += vertex.Normal * Bary[i];
	}
	Vert.position = WorldRayOrigin() + (WorldRayDirection() * RayTCurrent());
	return Vert;
}