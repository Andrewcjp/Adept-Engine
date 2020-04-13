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
#define READENCODED 1

#if READENCODED
Texture3D<uint4> voxelTex;
#else
Texture3D<uint> voxelTex;
#endif
VSOut main(uint vertexID : SV_VERTEXID)
{
	VSOut o;

	uint3 coord = unflatten3D(vertexID, VoxelRes);
	o.pos = float4(coord, 1);
#if READENCODED
	uint4 vox = voxelTex[coord];
	o.col = float4(DecodeColor(vox.r).xyz, vox.a);
#else
	uint vox = voxelTex[coord];
	o.col = float4(1, 1, 1, vox);
#endif
	uint3 maxv = VoxelRes - 1;
	if (coord.x == 0 && coord.y == 0 && coord.z == 0)
	{
		o.col = float4(0, 1, 0, 1);
	}
	else if (coord.x == maxv.x && coord.y == maxv.y && coord.z == maxv.z)
	{
		o.col = float4(1, 0, 0, 1);
	}
	return o;
}