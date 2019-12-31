#include "Voxel/VoxelCommon.hlsl"
struct GSOutput
{
	float4 pos : SV_POSITION;
	float4 col : TEXCOORD;
};
inline float3 CreateCube(in uint vertexID)
{
	uint b = 1 << vertexID;
	return float3((0x287a & b) != 0, (0x02af & b) != 0, (0x31e3 & b) != 0);
}
cbuffer SceneConstantBuffer : register(b2)
{
	row_major matrix View;
	row_major matrix Projection;
};
[maxvertexcount(14)]
void main(
	point GSOutput input[1],
	inout TriangleStream< GSOutput > output
)
{
	[branch]
	if (input[0].col.a > 0)
	{
		for (uint i = 0; i < 14; i++)
		{
			GSOutput element;
			element.pos = input[0].pos;
			element.col = input[0].col;

			element.pos.xyz = element.pos.xyz / VoxelRes * 2 - 1;
			element.pos.y = -element.pos.y;
			element.pos.xyz *= VoxelRes;
			element.pos.xyz += (CreateCube(i) - float3(0, 1, 0)) * 2;
			element.pos.xyz *= VoxelRes * VoxelSize / VoxelRes;
			row_major float4x4 model = float4x4(1, 0, 0, 0
				, 0, 1, 0, 0
				, 0, 0, 1, 0
				, 0, 0, 0, 1);
			float4 pos = float4(element.pos.xyz, 1);

			pos = mul(pos, model);
			pos = mul(pos, View);
			pos = mul(pos, Projection);
			element.pos = pos;
			output.Append(element);
		}
		output.RestartStrip();
	}
}