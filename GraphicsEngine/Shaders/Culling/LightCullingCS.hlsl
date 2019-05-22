RWStructuredBuffer<uint> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
groupshared uint LightIndexs[MAX_LIGHTS];
groupshared uint ArrayLength;
#include "Lighting.hlsl"
#include "Common.hlsl"
cbuffer LightBuffer : register(b1)
{
	int LightCount;
	int2 TileCount;
	Light lights[MAX_LIGHTS];
};

void AppendEntity(uint entityIndex)
{
	uint index;
	InterlockedAdd(ArrayLength, 1, index);
	if (index < MAX_LIGHTS)
	{
		LightIndexs[index] = entityIndex;
	}
}
inline uint flatten2D(uint2 coord, uint2 dim)
{
	return coord.x + coord.y * dim.x;
}
[numthreads(LIGHTCULLING_TILE_SIZE, LIGHTCULLING_TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 DGid : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint  groupIndex : SV_GroupIndex)
{
	if (groupIndex == 0)
	{
		ArrayLength = 0;
	}
	//build AABB
	//debug culling for now
	for (int i = groupIndex; i < MAX_LIGHTS; i += LIGHTCULLING_TILE_SIZE * LIGHTCULLING_TILE_SIZE)
	{		
		if (length(lights[i].color) > 0)
		{
			AppendEntity(i);
		}
	}
	GroupMemoryBarrierWithGroupSync();
	//write to Buffer
	int index = flatten2D(groupID.xy, TileCount)*MAX_LIGHTS;
	if (groupIndex == 0)
	{
		DstTexture[index] = ArrayLength;
	}
	GroupMemoryBarrierWithGroupSync();
	for (int i = groupIndex; i < MAX_LIGHTS - 1; i += LIGHTCULLING_TILE_SIZE * LIGHTCULLING_TILE_SIZE)
	{
		if (i > ArrayLength)
		{
			DstTexture[index + 1 + i] = -1;
		}
		else
		{
			DstTexture[index + 1 + i] = i;
		}
		
	}
}
