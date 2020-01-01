#include "Voxel/VoxelCommon.hlsl"
struct VSOut
{
	float4 pos : SV_POSITION;
	float4 col : TEXCOORD;
};
inline uint3 unflatten3D(uint idx, uint3 dim)
{
	const uint z = idx / (dim.x * dim.y);
	idx -= (z * dim.x * dim.y);
	const uint y = idx / dim.x;
	const uint x = idx % dim.x;
	return  uint3(x, y, z);
}
Texture3D<uint4> voxelTex;
VSOut main(uint vertexID : SV_VERTEXID)
{
	VSOut o;

	uint3 coord = unflatten3D(vertexID, VoxelRes);
	o.pos = float4(coord, 1);
	uint4 vox = voxelTex[coord];
	o.col = float4(DecodeColor(vox.r).xyz, vox.a);
	uint maxv = VoxelRes - 1;
	if (coord.x == 0 && coord.y == 0 && coord.z == 0)
	{
		o.col = float4(0, 1, 0, 1);
	}
	else if (coord.x == maxv && coord.y == maxv && coord.z == maxv)
	{
		o.col = float4(1, 0, 0, 1);
	}
	return o;
}