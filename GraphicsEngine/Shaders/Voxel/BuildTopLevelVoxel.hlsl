#include "Voxel/VoxelCommon.hlsl"
struct InstanceDesc
{
	float4x4 Transform;
	float4 Position;
	uint4 Size;
	float4 HalfExtends;
	int Index;
};

StructuredBuffer<InstanceDesc> Descs : register(t0);
Texture3D<uint4> VoxelData : register(t1);
cbuffer ControlData :register(b0)
{
	int DescCount;
};
RWTexture3D<uint4> TopLevelStructure : register(u0);
RWTexture3D<float> TopLevelAlphaMap : register(u1);

bool VectorEqual(float3 A, float3 B)
{
	return A.x == B.x && A.y == B.y && A.z == B.z;
}
PUSHCONST cbuffer IndexData : register(b0)
{
	int3 Offset;
};

uint DetmineA(uint3 pos)
{
	uint4 Sample = VoxelData[pos.xyz];
	return Sample.a;
}

[numthreads(8, 8, 8)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint3 Pos = Offset + DTid.xyz;
	uint4 Sample = VoxelData[DTid];

	if (Sample.a > 0)
	{
		TopLevelStructure[Pos] = Sample;
		TopLevelAlphaMap[Pos] = Sample.a;
	}
	TopLevelStructure[uint3(50, 50, 50)] = float4(1, 1, 1, 1);
	TopLevelStructure[uint3(50, 50-20, 50)] = float4(1, 1, 1, 1);
}