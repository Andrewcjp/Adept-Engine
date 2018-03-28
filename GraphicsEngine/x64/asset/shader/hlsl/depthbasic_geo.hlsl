struct GSPS_INPUT
{
	float4 Pos : SV_POSITION;
};
cbuffer ConstantBuffer : register(b0)
{
	row_major matrix world[6];
}

struct GSOutput
{
	float4 Pos : SV_Position;
	uint slice : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void main(triangleadj GSPS_INPUT input[6], inout TriangleStream<GSOutput> OutputStream)
{
	GSOutput output = (GSOutput)0;

	for (uint face = 0; face<6; face += 1)//for each face
	{
		for (int i = 0; i < 3; ++i) // for each triangle's vertices
		{
			output.Pos = mul(input[i].Pos, world[face]);
			output.slice = face;
			OutputStream.Append(output);
		}	
		OutputStream.RestartStrip();
	}
	
}

