#define SUPPORT_VRR 1
#include "VRX/VRRCommon.hlsl"
#include "Utils/UavLoad.hlsl"
Texture2D<float4> SrcTexture: register(t3);
Texture2D<uint> RateImage: register(t0);
RWTexture2D<float4>  DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
StructuredBuffer<uint4> TileList : register(t2);
RWByteAddressBuffer TileData : register(u10);
cbuffer Data : register(b1)
{
	int2 Resolution;
	float LerpBlend;
	bool DebugShow;
	bool DebugShowLines;
};
groupshared int4 AddressCache[VRS_TILE_SIZE*VRS_TILE_SIZE];
groupshared uint BufferLength = 0;
#define VRR_BLEND 1
#define BUILD_SHIPPING 0
#ifndef PS_RESOLVE
[numthreads(VRS_TILE_SIZE, VRS_TILE_SIZE, 1)]
void main(uint3 DTid : SV_GroupThreadID, uint3 groupIndex : SV_GroupID)
{
	if (DTid.x == 0 && DTid.y == 0)
	{
		BufferLength = 0;
	}
	GroupMemoryBarrierWithGroupSync();
	int StartIndex = groupIndex.x*TILES_PER_WARP;
	uint VarTileCount = TileData.Load(TILE_HEADER_OFFSET_VARTILES);

	const uint4 TileData = TileList[StartIndex];
	uint2 Pixel = TileData.xy * VRS_TILE_SIZE;
	const uint2 Rate = TileData.zw;

	Pixel += DTid.xy;

	[branch]
	if (!IsPixelSource(Pixel.xy, Rate))
	{
		//find the corse pixel for this pixel
		const int2 DeltaToMain = Pixel.xy % Rate.xy;
		const int2 SourcePixel = Pixel.xy - DeltaToMain;
		//DstTexture[Pixel.xy] = DstTexture[SourcePixel];
		uint index;
		InterlockedAdd(BufferLength, 1, index);
		AddressCache[index] = int4(SourcePixel, Pixel.xy);
	}
	GroupMemoryBarrierWithGroupSync();
	uint Pos = DTid.x + DTid.y*VRS_TILE_SIZE;
	if (Pos < BufferLength)
	{
		int4 Data = AddressCache[Pos];
		DstTexture[Data.zw] = DstTexture[Data.xy];
	}
	//DstTexture[Pixel.xy] = float4(BufferLength / 50, 0, 0, 1);
}
#endif


