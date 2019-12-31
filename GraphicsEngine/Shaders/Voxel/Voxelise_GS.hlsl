#include "Voxel/VoxelCommon.hlsl"
struct GSInput
{
	float4 position : SV_POSITION;
	float4 Normal :NORMAL0;
	float2 uv : TEXCOORD;
	float4 WorldPos:TANGENT0;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 Normal : NORMAL;
	float3 P : POSITION3D;
};

[maxvertexcount(3)]
void main(triangle GSInput input[3], inout TriangleStream< GSOutput > outputStream)
{
	GSOutput output[3];

	float3 facenormal = abs(input[0].Normal + input[1].Normal + input[2].Normal);
	uint maxi = facenormal[1] > facenormal[0] ? 1 : 0;
	maxi = facenormal[2] > facenormal[maxi] ? 2 : maxi;

	[unroll]
	for (uint i = 0; i < 3; ++i)
	{
		// World space -> Voxel grid space:
		output[i].pos.xyz = (input[i].WorldPos.xyz - VoxelGridCenter) * VoxelSize_INV;

		// Project onto dominant axis:
		[flatten]
		if (maxi == 0)
		{
			output[i].pos.xyz = output[i].pos.zyx;
		}
		else if (maxi == 1)
		{
			output[i].pos.xyz = output[i].pos.xzy;
		}
	}

#if 0
	//Conservative Rasterization for GPUs that don't have support
	float2 side0N = normalize(output[1].pos.xy - output[0].pos.xy);
	float2 side1N = normalize(output[2].pos.xy - output[1].pos.xy);
	float2 side2N = normalize(output[0].pos.xy - output[2].pos.xy);
	output[0].pos.xy += normalize(side2N - side0N);
	output[1].pos.xy += normalize(side0N - side1N);
	output[2].pos.xy += normalize(side1N - side2N);
#endif
	
	[unroll]
	for (uint j = 0; j < 3; j++)
	{
		// Voxel grid space -> Clip space
		output[j].pos.xy *= VoxelRes_INV;
		output[j].pos.zw = 1;
		output[j].uv = input[j].uv;
		output[j].Normal = input[j].Normal;
		output[j].P = input[j].WorldPos.xyz;
		outputStream.Append(output[j]);
	}

	outputStream.RestartStrip();
}