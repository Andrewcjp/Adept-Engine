RWStructuredBuffer<uint> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);
groupshared uint LightIndexs[MAX_LIGHTS];
groupshared uint ArrayLength;
#include "Lighting.hlsl"
#include "Core/Common.hlsl"
#include "CullingCommon.hlsl"
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
	//check spheres aganist it.

	const float3 eyePos = float3(0, 0, 0);

	// Compute 4 points on the far clipping plane to use as the 
	// frustum vertices.
	uint2 ID = DTid.xy;
	int Size = LIGHTCULLING_TILE_SIZE;//LIGHTCULLING_TILE_SIZE
	float4 screenSpace[4];
	// Top left point
	screenSpace[0] = float4(ID.xy * Size, 1.0f, 1.0f);
	// Top right point
	screenSpace[1] = float4(float2(ID.x + 1, ID.y) * Size, 1.0f, 1.0f);
	// Bottom left point
	screenSpace[2] = float4(float2(ID.x, ID.y + 1) * Size, 1.0f, 1.0f);
	// Bottom right point
	screenSpace[3] = float4(float2(ID.x + 1, ID.y + 1) * Size, 1.0f, 1.0f);

	float3 viewSpace[4];
	// Now convert the screen space points to view space
	for (int i = 0; i < 4; i++)
	{
		viewSpace[i] = ScreenToView(screenSpace[i]).xyz;
	}

	// Now build the frustum planes from the view space points
	Frustum frustum;

	// Left plane
	frustum.planes[0] = ComputePlane(viewSpace[2], eyePos, viewSpace[0]);
	// Right plane
	frustum.planes[1] = ComputePlane(viewSpace[1], eyePos, viewSpace[3]);
	// Top plane
	frustum.planes[2] = ComputePlane(viewSpace[0], eyePos, viewSpace[1]);
	// Bottom plane
	frustum.planes[3] = ComputePlane(viewSpace[3], eyePos, viewSpace[2]);
	float fMaxDepth = 1000.0f;
	float maxDepthVS = ScreenToView(float4(0, 0, fMaxDepth, 1)).z;
	float nearClipVS = ScreenToView(float4(0, 0, 1, 1)).z;
	//debug culling for now 
	for (int i = groupIndex; i < MAX_LIGHTS; i += LIGHTCULLING_TILE_SIZE * LIGHTCULLING_TILE_SIZE)
	{
		if (length(LightList[i].color) > 0)
		{
			float3 PosVS = mul(float4(LightList[i].LPosition, 0.0f), View);
			Sphere Sp = { PosVS ,10 };
			//float dis = length(Testpos.xyz - PosVS);
			//if (SphereInsideFrustum(Sp, frustum, nearClipVS, maxDepthVS))
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
