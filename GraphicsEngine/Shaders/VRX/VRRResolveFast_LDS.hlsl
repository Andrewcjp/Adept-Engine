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
#define CPU_CACHE 0
#if CPU_CACHE
cbuffer AddressData: register(b2)
{
	int4 AddressCache[VRS_TILE_SIZE*VRS_TILE_SIZE];
	int4 BufferLength;
}
#else
groupshared int4 AddressCache[VRS_TILE_SIZE*VRS_TILE_SIZE];
groupshared uint BufferLength = 0;
#endif
#define VRR_BLEND 1
#define BUILD_SHIPPING 0
#ifndef PS_RESOLVE
[numthreads(VRS_TILE_SIZE, VRS_TILE_SIZE, 1)]
void main(uint3 DTid : SV_GroupThreadID, uint3 groupIndex : SV_GroupID)
{
#if !CPU_CACHE
	if (DTid.x == 0 && DTid.y == 0)
	{
		BufferLength = 0;
	}
	GroupMemoryBarrierWithGroupSync();
#endif
	int StartIndex = groupIndex.x*TILES_PER_WARP;
	uint VarTileCount = TileData.Load(TILE_HEADER_OFFSET_VARTILES);

	const uint4 TileData = TileList[StartIndex];
	uint2 Pixel = TileData.xy * VRS_TILE_SIZE;
	const uint2 Rate = TileData.zw;

	Pixel += DTid.xy;

#if !CPU_CACHE
	[branch]
	if (!IsPixelSource(Pixel.xy, Rate))
	{
		//find the corse pixel for this pixel
		const int2 DeltaToMain = Pixel.xy % Rate.xy;
		const int2 SourcePixel = Pixel.xy - DeltaToMain;
		//DstTexture[Pixel.xy] = DstTexture[SourcePixel];
		uint index;
		InterlockedAdd(BufferLength, 1, index);
		AddressCache[index] = int4(-DeltaToMain, DTid.xy);
	}
	GroupMemoryBarrierWithGroupSync();
#endif
	//int4 Adr[TILES_PER_WARP];
	int i = 0;
#if TILES_PER_WARP > 1
	[unroll(TILES_PER_WARP)]
	for (;i < TILES_PER_WARP; i++)
#endif
	{
#if TILES_PER_WARP > 1
		if (StartIndex + i >= VarTileCount)
		{
			continue;
		}
#endif
		uint2 TargetPixel = TileList[StartIndex+i].xy * VRS_TILE_SIZE;
		uint Pos = DTid.x + DTid.y*VRS_TILE_SIZE;

		if (Pos < BufferLength.x)
		{
			int4 Data = AddressCache[Pos];
			int2 SourcePx = (TargetPixel.xy) + Data.zw;
			int2 DstPixel = TargetPixel.xy + Data.zw + Data.xy;
			DstTexture[SourcePx] = DstTexture[DstPixel];
			//Adr[i] = int4(SourcePx.xy, DstPixel.xy);
		}
	}
#if 0
	//DstTexture[Pixel.xy] = float4(BufferLength / 50, 0, 0, 1);
	float4 V1 = DstTexture[Adr[0].zw];
	float4 V2 = DstTexture[Adr[1].zw];
	float4 V3 = DstTexture[Adr[2].zw];
	float4 V4 = DstTexture[Adr[3].zw];

	DstTexture[Adr[0].xy] = V1;
	DstTexture[Adr[1].xy] = V2;
	DstTexture[Adr[2].xy] = V3;
	DstTexture[Adr[3].xy] = V4;
#endif
}
#endif


