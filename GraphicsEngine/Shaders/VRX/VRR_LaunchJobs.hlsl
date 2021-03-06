#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
RWStructuredBuffer<uint2> TileList_VAR: register(u0);
RWByteAddressBuffer TileData : register(u1);
RWByteAddressBuffer IndirectCommandBuffer_VAR: register(u2);


[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	uint VarTileCount = TileData.Load(TILE_HEADER_OFFSET_VARTILES);
	int Add = VarTileCount % TILES_PER_WARP;
	IndirectCommandBuffer_VAR.Store(0, (VarTileCount / TILES_PER_WARP) + Add);
	IndirectCommandBuffer_VAR.Store(4, 1);
	IndirectCommandBuffer_VAR.Store(8, 1);

	TileData.Store(0, 0);
	TileData.Store(TILE_HEADER_OFFSET_VARTILES, 0);
}