#include "Voxel/VoxelCommon.hlsl"
inline bool is_saturated(float a) { return a == saturate(a); }
inline float4 ConeTrace(in Texture3D<float4> voxels, in float3 P, in float3 N, in float3 coneDirection, in float coneAperture)
{
	float3 color = 0;
	float alpha = 0;

	// We need to offset the cone start position to avoid sampling its own voxel (self-occlusion):
	//	Unfortunately, it will result in disconnection between nearby surfaces :(
	float dist = VoxelSize; // offset by cone dir so that first sample of all cones are not the same
	float3 startPos = P + N * VoxelSize * 2 * SQRT2; // sqrt2 is diagonal voxel half-extent

	// We will break off the loop if the sampling distance is too far for performance reasons:
	while (dist < VoxelMaxDistance && alpha < 1)
	{
		float diameter = max(VoxelSize, 2 * coneAperture * dist);
		float mip = log2(diameter * VoxelSize_INV);

		// Because we do the ray-marching in world space, we need to remap into 3d texture space before sampling:
		//	todo: optimization could be doing ray-marching in texture space
		float3 tc = startPos + coneDirection * dist;
		tc = (tc - VoxelGridCenter) * VoxelSize_INV;
		tc *= VoxelRes_INV;
		tc = tc * float3(0.5f, -0.5f, 0.5f) + 0.5f;

		// break if the ray exits the voxel grid, or we sample from the last mip:
		if (!is_saturated(tc) /*|| mip >= (float)VoxelRadianceDataMIPs*/)
			break;

		float4 sam = voxels.SampleLevel(g_Clampsampler, tc, 0);

		// this is the correct blending to avoid black-staircase artifact (ray stepped front-to back, so blend front to back):
		float a = 1 - alpha;
		color += a * sam.rgb;
		alpha += a * sam.a;

		// step along ray:
		dist += diameter * VoxelRayStepDistance;
	}

	return float4(color, alpha);
}
struct TracePayload
{
	float3 color;
	float3 Normal;
	float3 pos;
	float alpha;
};
//more ray trace but
inline TracePayload ConeTrace_FixedMip(in Texture3D<uint4> voxels, in float3 P, in float3 N, in float3 coneDirection)
{
	float3 color = 0;
	float alpha = 0;

	// We need to offset the cone start position to avoid sampling its own voxel (self-occlusion):
	//	Unfortunately, it will result in disconnection between nearby surfaces :(
	float dist = VoxelSize; // offset by cone dir so that first sample of all cones are not the same
	float3 startPos = P + N * VoxelSize * 2 * SQRT2; // sqrt2 is diagonal voxel half-extent
	int hitcount = 0;
	TracePayload PayLoad;
	// We will break off the loop if the sampling distance is too far for performance reasons:
	while (dist < VoxelMaxDistance && alpha < 1)
	{		
		// Because we do the ray-marching in world space, we need to remap into 3d texture space before sampling:
		//	todo: optimization could be doing ray-marching in texture space
		float3 tc = startPos + coneDirection * dist;
		tc = (tc - VoxelGridCenter) /** VoxelSize_INV*/;
		tc *= VoxelRes_INV;
		tc = tc * float3(0.5f, -0.5f, 0.5f) + 0.5f;

		// break if the ray exits the voxel grid, or we sample from the last mip:
		if (!is_saturated(tc))
		{
			PayLoad.alpha = 0.0f;
			PayLoad.Normal = coneDirection;
			break;
		}

		uint4 sam = voxels[tc*VoxelRes];

		if (sam.a > 0)
		{
			PayLoad.color = GetPackedCol(sam);
			PayLoad.Normal = GetPackedNormal(sam);
			PayLoad.pos = startPos + coneDirection * dist;
			PayLoad.alpha = 1.0f;
			break;
		}
		// step along ray:
		dist += VoxelRayStepDistance;
	}

	return PayLoad;
}

float3 ComputePoint(float3 startPos, float3 Dir, float dist)
{
	float3 tc = startPos + Dir * dist;
	tc = (tc - VoxelGridCenter) /** VoxelSize_INV*/;
	tc *= VoxelRes_INV;
	tc = tc * float3(0.5f, -0.5f, 0.5f) + 0.5f;
	return tc;
}
inline TracePayload ConeTrace_FixedMip_Loop2(in Texture3D<uint4> voxels, in float3 P, in float3 N, in float3 coneDirection)
{
	// We need to offset the cone start position to avoid sampling its own voxel (self-occlusion):
	//	Unfortunately, it will result in disconnection between nearby surfaces :(
	float dist = VoxelSize; // offset by cone dir so that first sample of all cones are not the same
	float3 startPos = P + N * VoxelSize * 2 * SQRT2; // sqrt2 is diagonal voxel half-extent
	int hitcount = 0;
	TracePayload PayLoad;
	PayLoad.Normal = coneDirection;
	PayLoad.alpha = 0.0f;
	// We will break off the loop if the sampling distance is too far for performance reasons:

	while (dist < VoxelMaxDistance)
	{
		// Because we do the ray-marching in world space, we need to remap into 3d texture space before sampling:
		//	todo: optimization could be doing ray-marching in texture space
		float3 tc = ComputePoint(startPos, coneDirection, dist);
		float3 tc2 = ComputePoint(startPos, coneDirection, dist+ VoxelRayStepDistance);
		// break if the ray exits the voxel grid, or we sample from the last mip:
		if (!is_saturated(tc) || !is_saturated(tc2))
		{
			PayLoad.alpha = 0.0f;
			PayLoad.color = float3(0,0,0);
			PayLoad.pos = startPos + coneDirection * dist;
			PayLoad.Normal = coneDirection;
			break;
		}

		uint4 sam = voxels[tc*VoxelRes];
		uint4 sam2 = voxels[tc*VoxelRes];
		if (sam.a > 0)
		{
			
		}
		else if(sam2.a > 0)
		{
			sam = sam2;
		}
		if (sam.a > 0)
		{
			PayLoad.color = GetPackedCol(sam);
			PayLoad.Normal = GetPackedNormal(sam);
			PayLoad.pos = startPos + coneDirection * dist;
			PayLoad.alpha = 1.0f;
			break;
		}
		// step along ray:
		dist += VoxelRayStepDistance*2;
	}

	return PayLoad;
}
//1     2       4
//495	350		243
//      70%	    69% (50%)
inline TracePayload ConeTrace_FixedMip_Var(in Texture3D<uint4> voxels, in float3 P, in float3 N, in float3 coneDirection)
{
	// We need to offset the cone start position to avoid sampling its own voxel (self-occlusion):
	//	Unfortunately, it will result in disconnection between nearby surfaces :(
	float dist = VoxelSize; // offset by cone dir so that first sample of all cones are not the same
	float3 startPos = P + N * VoxelSize * 2 * SQRT2; // sqrt2 is diagonal voxel half-extent
	int hitcount = 0;
	TracePayload PayLoad;
	// We will break off the loop if the sampling distance is too far for performance reasons:
	const int LoopCount = 4;
	while (dist < VoxelMaxDistance)
	{
		// Because we do the ray-marching in world space, we need to remap into 3d texture space before sampling:
		//	todo: optimization could be doing ray-marching in texture space
		float3 tc[LoopCount];
		uint4 sam[LoopCount];
		[unroll(LoopCount)]
		for (int i = 0; i < LoopCount; i++)
		{
			tc[i] = ComputePoint(startPos, coneDirection, dist + VoxelRayStepDistance * i);
			// break if the ray exits the voxel grid, or we sample from the last mip:
			if (!is_saturated(tc[i]))
			{
				PayLoad.alpha = 0.0f;
				PayLoad.color = float3(0, 0, 0);
				PayLoad.pos = startPos + coneDirection * dist + VoxelRayStepDistance * i;
				PayLoad.Normal = coneDirection;
				return PayLoad;
			}
		}
		[unroll(LoopCount)]
		for (int i = 0; i < LoopCount; i++)
		{			
			sam[i] = voxels[tc[i] * VoxelRes];
		}
		[unroll(LoopCount)]
		for (int i = 0; i < LoopCount; i++)
		{
			if (sam[i].a > 0)
			{
				PayLoad.color = GetPackedCol(sam[i]);
				PayLoad.Normal = GetPackedNormal(sam[i]);
				PayLoad.pos = startPos + coneDirection * dist + VoxelRayStepDistance * i;
				PayLoad.alpha = 1.0f;
				return PayLoad;
			}
		}
		// step along ray:
		dist += VoxelRayStepDistance * LoopCount;
	}

	return PayLoad;
}