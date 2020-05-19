#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"

Texture2D<uint> RateImage: register(t0);
RWStructuredBuffer<uint4> TileList_VAR: register(u0);
RWByteAddressBuffer TileData : register(u1);


[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	//if (DTid.x == 0 && DTid.y == 0)
	//{
	//	TileData.Store(0, 0);
	//	TileData.Store(TILE_HEADER_OFFSET_VARTILES, 0);
	//}
	//GroupMemoryBarrierWithGroupSync();
	uint Rate = RateImage[DTid.xy];
	uint LastCount = 0;
#if 1
	if (Rate == SHADING_RATE_1X1)
	{
		TileData.InterlockedAdd(TILE_HEADER_OFFSET_NOP, 1, LastCount);
	}
#endif
	if (Rate != SHADING_RATE_1X1)
	{
		TileData.InterlockedAdd(TILE_HEADER_OFFSET_VARTILES, 1, LastCount);
		TileList_VAR[LastCount] = uint4(DTid.xy, GetShadingRate(Rate).xy);
	}
}