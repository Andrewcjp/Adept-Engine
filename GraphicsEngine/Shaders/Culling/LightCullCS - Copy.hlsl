#include "../Lighting.hlsl"
#include "../Core/Common.hlsl"
//#include "CullingCommon.hlsl"
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
groupshared uint LightIndexs[MAX_LIGHTS_PER_TILE];
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
struct Sphere
{
	float3 Pos;	 // Center point.
	float Radius;	// Radius.
};

bool SphereFrustumTestVS(Sphere sphere, Frustum frustum, float zNear, float zFar)
{
	bool result = true;
	result = ((sphere.c.z + sphere.r < zNear || sphere.Pos.z - sphere.raduis > zFar) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[0])) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[1])) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[2])) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[3])) ? false : result);
	return result;
}

bool TestLight(uint index, float MinZ, float MaxZ)
{
	float3 wp = LightList[index].LPosition;
	float4 PosVS = mul(float4(LightList[index].LPosition, 1.0f), View);
	/*PosVS = -PosVS;*/
	float minDepthVS = ScreenToView(float4(0, 0, MaxZ, 1)).z;
	float maxDepthVS = ScreenToView(float4(0, 0, MinZ, 1)).z;
	float nearClipVS = ScreenToView(float4(0, 0, 1, 1)).z;
	float Range = 10;// LightList[index].Range;
	if (PosVS.z - Range > maxDepthVS || PosVS.z + Range < minDepthVS)
	{
		return false;
	}
	/*if (MaxZ > 0.99)
	{
		return false;
	}*/
	//if (index != 0)
	//{
	//	return false;
	//}
	/*if (PosVS.z < -10 || PosVS.z >= 0)
	{
		return false;
	}*/
	return true;
}


[numThreads(LIGHTCULLING_TILE_SIZE, LIGHTCULLING_TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 DGid : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint  groupIndex : SV_GroupIndex)
{
	if (groupIndex == 0)
	{
		ListLength = 0;
		uMinZ = 0xffffffff;
		uMaxZ = 0;
	}
	GroupMemoryBarrierWithGroupSync();
	//if (groupIndex == 0)
	{
		float ZV = Depth[DTid.xy];
		uint Z = asuint(ZV);
		InterlockedMin(uMinZ, Z);
		InterlockedMax(uMaxZ, Z);
		//MaxZ = Z;
	}
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
	frustum.planes[0] = ComputePlane(viewSpacePoints[2], eyePos, viewSpacePoints[0]);
	frustum.planes[1] = ComputePlane(viewSpacePoints[1], eyePos, viewSpacePoints[3]);
	frustum.planes[2] = ComputePlane(viewSpacePoints[0], eyePos, viewSpacePoints[1]);
	frustum.planes[3] = ComputePlane(viewSpacePoints[3], eyePos, viewSpacePoints[2]);


	float minDepthVS = ScreenToView(float4(0, 0, ZMax, 1)).z;
	float maxDepthVS = ScreenToView(float4(0, 0, ZMin, 1)).z;
	for (int i = groupIndex; i < LightCount; i += LIGHTCULLING_TILE_SIZE * LIGHTCULLING_TILE_SIZE)
	{
		//test
		bool PassedTest = true;// TestLight(i, ZMin, ZMax);
		if (PassedTest)
		{
			float4 PosVS = mul(float4(LightList[i].LPosition, 1.0f), View);
			Sphere s = { PosVS.xyz, LightList[i].Range };
			PassedTest = SphereFrustumTest(s, frustum, minDepthVS, maxDepthVS);
		}
		if (PassedTest)
		{
			uint index;
			InterlockedAdd(ListLength, 1, index);
			LightIndexs[index] = i;
		}
	}
	GroupMemoryBarrierWithGroupSync();
	const int index = flatten2D(groupID.xy, TileCount)*MAX_LIGHTS_PER_TILE;
	//ArrayLength = 0;
	if (groupIndex == 0)
	{
		DstLightList[index] = ListLength;
	}

	GroupMemoryBarrierWithGroupSync();
	for (int i = groupIndex; i < MAX_LIGHTS_PER_TILE - 1; i += LIGHTCULLING_TILE_SIZE * LIGHTCULLING_TILE_SIZE)
	{
		if (i >= ListLength)
		{
			DstLightList[index + 1 + i] = -1;
		}
		else
		{
			DstLightList[index + 1 + i] = LightIndexs[i];
		}
	}
}