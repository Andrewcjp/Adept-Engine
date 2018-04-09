cbuffer GeoTrans : register(b1)
{
	row_major matrix worldm[6];
}
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	uint slice : SV_RenderTargetArrayIndex;
	float3 LightPos: NORMAL0;
};

struct GSOutput
{
	float4 Pos : SV_POSITION;
	uint slice : SV_RenderTargetArrayIndex;
	float3 LightPos: NORMAL0;
};

[maxvertexcount(18)]
void main(triangleadj VS_OUTPUT input[6], inout TriangleStream<GSOutput> OutputStream)
{
	for (uint face = 0; face < 6; face += 1)//for each face
	{
		GSOutput output = (GSOutput)0;
		for (int i = 0; i < 3; ++i) // for each triangle's vertices
		{
			output.Pos = mul(input[i].pos, worldm[face]);
			output.slice = face;
			output.LightPos = input[i].LightPos;
			OutputStream.Append(output);
		}
		OutputStream.RestartStrip();
	}

}

