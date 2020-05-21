#include "../Lighting.hlsl"
#include "../Core/Common.hlsl"
#include "Voxel/RayIntersect.hlsl"

RWStructuredBuffer<uint> DstLightList : register(u0);
StructuredBuffer<Light> LightList : register(t0);
Texture2D<float> Depth: register(t1);
cbuffer LightBuffer : register(b1)
{
	int LightCount;
	uint2 TileCount;
};
cbuffer CameraData : register(b2)
{
	row_major float4x4 View;
	row_major float4x4 Projection;
	float3 CameraPos;
	int2 Resolution;
	row_major float4x4 INV_Projection;
	float2 INV_Resolution;
};
#define MAX_LIGHTS_PER_TILE MAX_LIGHTS
groupshared uint ListLength;
groupshared uint LightIndexs[LIGHTCULLING_TILE_SIZE*LIGHTCULLING_TILE_SIZE];
groupshared uint uMinZ;
groupshared uint uMaxZ;
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
	float2 texCoord = screen.xy * INV_Resolution;

	// Convert to clip space
	float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);

	return ClipToView(clip);
}


struct Frustum
{
	Plane planes[4]; // MinZ and MaxZ handle Front and back planes.
};

Plane MakePlane(float3 p0, float3 p1, float3 p2)
{
	Plane plane;

	float3 v0 = p1 - p0;
	float3 v2 = p2 - p0;

	plane.Normal = normalize(cross(v0, v2));

	// Compute the distance to the origin using p0.
	plane.Dist = dot(plane.Normal, p0);

	return plane;
}

bool SphereInsidePlane(Sphere sphere, Plane plane)
{
	return dot(plane.Normal, sphere.Pos) - plane.Dist < -sphere.Radius;
}

bool SphereFrustumTestVS(Sphere sphere, Frustum frustum, float zNear, float zFar)
{
	bool result = true;
	result = ((sphere.Pos.z + sphere.Radius < zNear || sphere.Pos.z - sphere.Radius > zFar) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[0])) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[1])) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[2])) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[3])) ? false : result);
	return result;
}

void SortLights(in uint localIdxFlattened)
{
	uint numArray = ListLength;

	//// Round the number of entities up to the nearest power of two
	//uint numArrayPowerOfTwo = 1;
	//while( numArrayPowerOfTwo < numArray )
	//	numArrayPowerOfTwo <<= 1;

	// Optimized nearest power of two algorithm:
	uint numArrayPowerOfTwo = 2 << firstbithigh(numArray - 1); // if numArray is 0, we don't even call this function, but watch out!

	for (uint nMergeSize = 2; nMergeSize <= numArrayPowerOfTwo; nMergeSize = nMergeSize * 2)
	{
		for (uint nMergeSubSize = nMergeSize >> 1; nMergeSubSize > 0; nMergeSubSize = nMergeSubSize >> 1)
		{
			uint tmp_index = localIdxFlattened;
			uint index_low = tmp_index & (nMergeSubSize - 1);
			uint index_high = 2 * (tmp_index - index_low);
			uint index = index_high + index_low;

			uint nSwapElem = nMergeSubSize == nMergeSize >> 1 ? index_high + (2 * nMergeSubSize - 1) - index_low : index_high + nMergeSubSize + index_low;
			if (nSwapElem < numArray && index < numArray)
			{
				if (LightIndexs[index] < LightIndexs[nSwapElem])
				{
					uint uTemp = LightIndexs[index];
					LightIndexs[index] = LightIndexs[nSwapElem];
					LightIndexs[nSwapElem] = uTemp;
				}
			}
			GroupMemoryBarrierWithGroupSync();
		}
	}
}

[numthreads(LIGHTCULLING_TILE_SIZE, LIGHTCULLING_TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 DGid : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint  groupIndex : SV_GroupIndex)
{
	uint Z = asuint(Depth[DTid.xy]);
	if (groupIndex == 0)
	{
		ListLength = 0;
		uMinZ = 0xffffffff;
		uMaxZ = 0;
	}
	GroupMemoryBarrierWithGroupSync();

	InterlockedMin(uMinZ, Z);
	InterlockedMax(uMaxZ, Z);

	GroupMemoryBarrierWithGroupSync();
	//Construct Fustrum
	float ZMax = asfloat(uMinZ);
	float ZMin = asfloat(uMaxZ);
	float4 TilePoints[4];
	// Top left point
	TilePoints[0] = float4(groupID.xy * LIGHTCULLING_TILE_SIZE, 1.0f, 1.0f);
	// Top right point
	TilePoints[1] = float4(float2(groupID.x + 1, groupID.y) * LIGHTCULLING_TILE_SIZE, 1.0f, 1.0f);
	// Bottom left point
	TilePoints[2] = float4(float2(groupID.x, groupID.y + 1) * LIGHTCULLING_TILE_SIZE, 1.0f, 1.0f);
	// Bottom right point
	TilePoints[3] = float4(float2(groupID.x + 1, groupID.y + 1) * LIGHTCULLING_TILE_SIZE, 1.0f, 1.0f);

	float3 viewSpacePoints[4];
	// Now convert the screen space points to view space
	[unroll(4)]
	for (int i = 0; i < 4; i++)
	{
		viewSpacePoints[i] = ScreenToView(TilePoints[i]).xyz;
	}

	// Now build the frustum planes from the view space points
	Frustum frustum;
	float3 eyePos = float3(0, 0, 0);
	frustum.planes[0] = MakePlane(viewSpacePoints[2], eyePos, viewSpacePoints[0]);
	frustum.planes[1] = MakePlane(viewSpacePoints[1], eyePos, viewSpacePoints[3]);
	frustum.planes[2] = MakePlane(viewSpacePoints[0], eyePos, viewSpacePoints[1]);
	frustum.planes[3] = MakePlane(viewSpacePoints[3], eyePos, viewSpacePoints[2]);

	float3 AABPoints[8];
	AABPoints[0] = ScreenToView(float4(groupID.xy * LIGHTCULLING_TILE_SIZE, ZMin, 1.0f));
	AABPoints[1] = ScreenToView(float4(float2(groupID.x + 1, groupID.y)  * LIGHTCULLING_TILE_SIZE, ZMin, 1.0f));
	AABPoints[2] = ScreenToView(float4(float2(groupID.x, groupID.y + 1) * LIGHTCULLING_TILE_SIZE, ZMin, 1.0f));
	AABPoints[3] = ScreenToView(float4(float2(groupID.x + 1, groupID.y + 1) * LIGHTCULLING_TILE_SIZE, ZMin, 1.0f));

	AABPoints[4] = ScreenToView(float4(groupID.xy * LIGHTCULLING_TILE_SIZE, ZMax, 1.0f));
	AABPoints[5] = ScreenToView(float4(float2(groupID.x + 1, groupID.y)  * LIGHTCULLING_TILE_SIZE, ZMax, 1.0f));
	AABPoints[6] = ScreenToView(float4(float2(groupID.x, groupID.y + 1) * LIGHTCULLING_TILE_SIZE, ZMax, 1.0f));
	AABPoints[7] = ScreenToView(float4(float2(groupID.x + 1, groupID.y + 1) * LIGHTCULLING_TILE_SIZE, ZMax, 1.0f));
	float3 AABBMin = 10000000;
	float3 AABBMax = -10000000;
	for (int i = 0; i < 8; i++)
	{
		AABBMin = min(AABBMin, AABPoints[i]);
		AABBMax = max(AABBMax, AABPoints[i]);
	}
	AABB TileAABB;
	TileAABB.MakeFromMinMax(AABBMin, AABBMax);

	float minZVS = ScreenToView(float4(0, 0, ZMin, 1)).z;
	float maxZVS = ScreenToView(float4(0, 0, ZMax, 1)).z;
	float nearClipVS = ScreenToView(float4(0, 0, 0, 1)).z;
	for (int i = groupIndex; i < LightCount; i += LIGHTCULLING_TILE_SIZE * LIGHTCULLING_TILE_SIZE)
	{
		float4 PosVS = mul(float4(LightList[i].LPosition, 1.0f), View);
		Sphere s = { PosVS.xyz, LightList[i].Range };
		//s.Radius = 10;
		bool PassedTest = SphereFrustumTestVS(s, frustum, nearClipVS, minZVS);
		if (PassedTest)
		{
			PassedTest = TileAABB.SphereIntersectsAABB(s);
		}
		//todo: 2.5D cull
		if (PassedTest)
		{
#if 1
			uint index;
			InterlockedAdd(ListLength, 1, index);
#else
			uint Count = WaveActiveCountBits(PassedTest);
			uint laneAppendOffset = WavePrefixCountBits(PassedTest);
			uint index;
			if (WaveIsFirstLane())
			{
				InterlockedAdd(ListLength, Count, index);
			}
			index = WaveReadLaneFirst(index);
			index += laneAppendOffset;
#endif
			LightIndexs[index] = i;
		}
	}
	GroupMemoryBarrierWithGroupSync();
	const int index = flatten2D(groupID.xy, TileCount)*MAX_LIGHTS_PER_TILE;
	//ArrayLength = 0;
	
	if (groupIndex == 0)
	{
		ListLength = min(ListLength, MAX_LIGHTS_PER_TILE-1);
		DstLightList[index] = ListLength;
	}
	//sort!
	//SortLights(groupIndex);
	GroupMemoryBarrierWithGroupSync();
	for (int i = groupIndex; i < MAX_LIGHTS_PER_TILE - 1; i += LIGHTCULLING_TILE_SIZE * LIGHTCULLING_TILE_SIZE)
	{
		if (i >= ListLength)
		{
			//DstLightList[index + 1 + i] = -1;
		}
		else
		{
			DstLightList[index + 1 + i] = LightIndexs[i];
		}
	}
}