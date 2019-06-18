RWStructuredBuffer<uint> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
groupshared uint LightIndexs[MAX_LIGHTS];
groupshared uint ArrayLength;
#include "Lighting.hlsl"
#include "Core/Common.hlsl"
cbuffer LightBuffer : register(b1)
{
	int LightCount;
	uint2 TileCount;
};
cbuffer CameraData : register(b2)
{
	float4x4 View;
	float4x4 Projection;
	float3 CameraPos;
	float4x4 INV_Projection;
	int2 Resolution;
	float2 INV_Resolution;
};

float4 ClipToView(float4 clip)
{
	// View space position.
	float4 view = mul(clip, INV_Projection);
	// Perspective projection.
	view = view / view.w;

	return view;
}
// Convert screen space coordinates to view space.
float4 ScreenToView(float4 screen)
{
	// Convert to normalized texture coordinates
	float2 texCoord = screen.xy *INV_Resolution;

	// Convert to clip space
	float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);

	return ClipToView(clip);
}


StructuredBuffer<Light> LightList : register(t0);
void AppendEntity(uint entityIndex)
{
	uint index;
	InterlockedAdd(ArrayLength, 1, index);
	if (index < MAX_LIGHTS)
	{
		LightIndexs[index] = entityIndex;
	}
}

[numthreads(LIGHTCULLING_TILE_SIZE, LIGHTCULLING_TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 DGid : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint  groupIndex : SV_GroupIndex)
{
	if (groupIndex == 0)
	{
		ArrayLength = 0;
	}
	//build fustrum
	float4 	Testpos = float4(float2(DTid.x+1, DTid.y+1)/* * LIGHTCULLING_TILE_SIZE*/, 1.0f, 1.0f);
	Testpos = ScreenToView(Testpos);
	//check spheres aganist it.

	//debug culling for now 
	for (int i = groupIndex; i < MAX_LIGHTS; i += LIGHTCULLING_TILE_SIZE * LIGHTCULLING_TILE_SIZE)
	{
		if (length(LightList[i].color) > 0)
		{
			float3 PosVS =  mul(float4(LightList[i].LPosition, 1.0f), View);
			float dis = length(Testpos.xyz - PosVS);
			if (dis <= LightList[i].Range)
			{
				AppendEntity(i);
			}
		}
	}
	GroupMemoryBarrierWithGroupSync();
	//write to Buffer
	const int index = flatten2D(groupID.xy, TileCount)*MAX_LIGHTS;
	//ArrayLength = 0;
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
