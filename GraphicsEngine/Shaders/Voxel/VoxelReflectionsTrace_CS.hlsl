#include "Shading/ReflectionsTrace.hlsl"
#include "../Lighting.hlsl"
#include "RayIntersect.hlsl"
Texture3D<uint4> voxelTex :  register(t8);
RWTexture2D<float4> gOutput : register(u0);
Texture2D<float4> GBUFFER_Normal : register(t5);
Texture2D<float4> GBUFFER_Pos : register(t6);
Texture2D<float4> GBUFFER_BaseSpec : register(t7);

Texture3D<float> VoxelTransferMap : register(t9);
TextureCube SpecularBlurMap: register(t11);

SamplerState Textures : register (s0);
SamplerState g_Clampsampler : register(s1);
#include "Voxel/VoxelTrace_PS.hlsl"
cbuffer ConstData: register(b0)
{
	float3 CameraPos;
	int2 Res;
	int FrameCount;
}
cbuffer RTBufferData : register(b2)
{
	float RT_RoughnessThreshold;
	float VX_MaxRoughness;
	float VX_MinRoughness;
	float VX_RT_BlendStart;
	float VX_RT_BlendEnd;
	float VX_RT_BlendFactor;
	int Max_VXRayCount;
	int Max_RTRayCount;
};
cbuffer LightBuffer : register(b1)
{
	int LightCount;
};
StructuredBuffer<Light> LightList : register(t20);
#define MAX_TILE_PER_RAY 5
struct RayTileData
{
	uint Hits;
	float3 Tiles[MAX_TILE_PER_RAY];
	float3 WorldPos[MAX_TILE_PER_RAY];
	uint Mip[MAX_TILE_PER_RAY];
};
void AddVoxelHit(float3 Pos, uint mip, float3 WorldPos, inout RayTileData TileData)
{
	//if (mip == 0)
	{
		TileData.Tiles[TileData.Hits] = Pos;
		TileData.WorldPos[TileData.Hits] = WorldPos;
		TileData.Mip[TileData.Hits] = mip;
		TileData.Hits++;
	}
}
//#if 0
////use 3 samples to determine source Voxel pos
//float x = voxels.SampleLevel(g_Clampsampler, tc + float3(VoxelRes_INV.x, 0, 0), 0);
//float y = voxels.SampleLevel(g_Clampsampler, tc + float3(0, -VoxelRes_INV.y, 0), 0);
//float z = voxels.SampleLevel(g_Clampsampler, tc + float3(0, 0, VoxelRes_INV.z), 0);
//float3 SrcPos = tc - float3(x, y, z);
//AddVoxelHit(SrcPos, mip, TileData);
//#endif
float3 ConvertToVoxelSpace(float3 WorldPos)
{
	float3 tc = (WorldPos - VoxelGridCenter) * VoxelSize_INV;
	tc *= VoxelRes_INV;
	tc = tc * float3(0.5f, -0.5f, 0.5f) + 0.5f;
	return tc;
}
#define MAX_APLHA_VALUES 10
struct AlphaAccum
{
	float Values[MAX_APLHA_VALUES];
	float3 Pos[MAX_APLHA_VALUES];
	uint mips[MAX_APLHA_VALUES];
	int Count;
	int MaxValue;
	void AddValue(float v, float3 WPos, uint mip)
	{
		if (v > Values[MaxValue] /*|| mip > mips[MaxValue]*/)
		{
			MaxValue = Count;
		}
		Values[Count] = v;
		Pos[Count] = WPos;
		mips[Count] = mip;
		Count++;

	}
	float GetMaxValue()
	{
		if (Count == 0)
		{
			return 0.0f;
		}
		return  Values[MaxValue];
	}
	void Init()
	{
		MaxValue = 0;
		Count = 0;
	}
};
#define USE_AABBS 0
inline float ExecuteConeTrace(in Texture3D<float> voxels, in float3 P, in float3 N, in float3 coneDirection, in float coneAperture, inout RayTileData TileData)
{
	float3 color = 0;
	float alpha = 0;
	const float VoxelRadianceDataMIPs = 2;
	// We need to offset the cone start position to avoid sampling its own voxel (self-occlusion):
	//	Unfortunately, it will result in disconnection between nearby surfaces :(
	float dist = VoxelSize * 2; // offset by cone dir so that first sample of all cones are not the same
	float3 startPos = P + N * VoxelSize * 2 * SQRT2; // sqrt2 is diagonal voxel half-extent
	AlphaAccum Accum;
	Accum.Init();
	// We will break off the loop if the sampling distance is too far for performance reasons:
	while (dist < VoxelMaxDistance && alpha < 1)
	{
		float diameter = max(VoxelSize, 2 * coneAperture * dist);
		float mip = 0;// log2(diameter * VoxelSize_INV);
		//mip = min(mip, 1);
		// Because we do the ray-marching in world space, we need to remap into 3d texture space before sampling:
		//	todo: optimization could be doing ray-marching in texture space
		float3 raypos = startPos + coneDirection * dist;
		float3 tc = (raypos - VoxelGridCenter) * VoxelSize_INV;
		tc *= VoxelRes_INV;
		tc = tc * float3(0.5f, -0.5f, 0.5f) + 0.5f;

		// break if the ray exits the voxel grid, or we sample from the last mip:
		if (!is_saturated(tc) || mip >= (float)VoxelRadianceDataMIPs)
			break;

		float Contsample = voxels.SampleLevel(g_Clampsampler, tc, mip);
		// this is the correct blending to avoid black-staircase artifact (ray stepped front-to back, so blend front to back):
		float a = 1 - alpha;
		alpha += a * Contsample;
		if (Contsample > 0.0f)
		{
			Accum.AddValue(Contsample, raypos, mip);
			//break;
		}
		// step along ray:
		dist += diameter * VoxelRayStepDistance;
	}
	//if (alpha > 0.3)
	{

		int Index = Accum.MaxValue;
		if (Accum.Count > 0)
		{
			float3 TargetPos = Accum.Pos[Index];
#if USE_AABBS
			if (Accum.Values[Index] >= 1.0)
#endif
			{
				float3 Pos = ConvertToVoxelSpace(TargetPos) * VoxelRes;
				AddVoxelHit(Pos, Accum.mips[Index], TargetPos, TileData);
			}
#if USE_AABBS
			else
			{
				const int MAXTESTS = 10;
				AABB Boxes[MAXTESTS];
				const float3 AABBSIZE = float3(VoxelSize, VoxelSize, VoxelSize);
				Boxes[0].Init(TargetPos + (coneDirection * VoxelSize), AABBSIZE);
				Boxes[1].Init(TargetPos + float3(VoxelSize, 0, 0), AABBSIZE);
				Boxes[2].Init(TargetPos + float3(VoxelSize, 0, VoxelSize), AABBSIZE);
				Boxes[3].Init(TargetPos + float3(0, 0, VoxelSize), AABBSIZE);

				Boxes[4].Init(TargetPos + float3(0, VoxelSize, 0), AABBSIZE);
				Boxes[5].Init(TargetPos + float3(0, -VoxelSize, 0), AABBSIZE);

				Boxes[6].Init(TargetPos + float3(VoxelSize, VoxelSize, VoxelSize), AABBSIZE);
				Boxes[7].Init(TargetPos + float3(-VoxelSize, -VoxelSize, -VoxelSize), AABBSIZE);

				Boxes[8].Init(TargetPos + float3(-VoxelSize, 0, 0), AABBSIZE);
				Boxes[9].Init(TargetPos + float3(0, 0, -VoxelSize), AABBSIZE);
				//transform by ray
				//add remaning cases
				//could cheat and just sample up 1 mip?
				Ray r;
				r.Origin = TargetPos;
				r.Dir = coneDirection;
				bool hit = false;
				for (int i = 0; i < MAXTESTS; i++)
				{
					RayHitResult HitInfo;
					hit = RayAABB(Boxes[i], r, HitInfo);
					if (hit)
					{
						float3 Pos = ConvertToVoxelSpace(Boxes[i].Position);
						if (voxels[Pos * VoxelRes] >= 1.0f)
						{
							AddVoxelHit(Pos* VoxelRes, 0, Pos, TileData);
							break;
						}
					}
				}
			}
#endif
		}
		else
		{
			//return 0.0f;
		}
	}
	return alpha;
}
float3 IntergrateHigherMip(in RayTileData TileData)
{

}
float3 LightVoxels(in RayTileData TileData)
{
	if (TileData.Hits == 0)
	{
		return float3(0, 0, 1);
	}
	float3 VoxelColour = float3(0, 0, 0);
	float3 Normal = float3(0, 1, 0);
	float3 WorldPos = float3(0, 0, 0);
	bool AnyHit = false;
	for (int i = 0; i < TileData.Hits; i++)
	{
		uint3 SamplePos = floor(TileData.Tiles[i]);
		uint4 Sample = voxelTex[SamplePos];
		if (Sample.a == 0)
		{
			continue;
		}
		if (TileData.Mip[i] != 0)
		{
			continue;
		}
		VoxelColour = GetPackedCol(Sample);
		Normal = GetPackedNormal(Sample);
		WorldPos = TileData.WorldPos[i];
		AnyHit = true;
		break;
	}
	const int MaxMip = 1;
	if (TileData.Mip[0] > MaxMip)
	{
		VoxelColour = float3(0, 0, 0);
		AnyHit = true;
	}
	int ClampedMip = min(MaxMip, TileData.Mip[0]);
	if (!AnyHit)
	{
		//sample one mip up!
		//128*128*128
		float3 StartPos = TileData.WorldPos[0];
		const float mipSize = 4 * (ClampedMip+1);
		const float halfSize = mipSize / 2;
		int SampleCount = 0;
		for (float x = -halfSize; x < halfSize; x++)
		{
			for (float y = -halfSize; y < halfSize; y++)
			{
				for (float z = -halfSize; z < halfSize; z++)
				{
					float3 SamplePos = StartPos + (float3(x, y, z)*VoxelSize);
					uint4 Sample = voxelTex[floor(ConvertToVoxelSpace(SamplePos)*VoxelRes)];
					if (Sample.a > 0)
					{
						VoxelColour += GetPackedCol(Sample);
						Normal += GetPackedNormal(Sample);
						AnyHit = true;
						SampleCount++;
						break;
					}
				}
			}
		}
		if (SampleCount)
		{
			VoxelColour /= SampleCount;
			Normal /= SampleCount;
		}
	}
	if (!AnyHit)
	{
		VoxelColour = float3(0, 1, 0);
	}
	float3 OutColor = VoxelColour.xyz *GetAmbient_CONST();
	//float3 CalcColorFromLight(Light light, float3 Diffusecolor, float3 FragPos, float3 normal, float3 CamPos, float roughness, float Metalic)
	for (int i = 0; i < LightCount; i++)
	{
		float3 LightColour = CalcColorFromLight(LightList[i], VoxelColour, WorldPos, Normal, CameraPos, 0.0f, 0.0f);
		/*if (LightList[i].HasShadow && LightList[i].type == 1)
		{
			LightColour *= 1.0 - ShadowCalculationCube(WorldPos, LightList[i], ShadowData[LightList[i].ShadowID]);
		}*/
		OutColor += LightColour;
	}
	return OutColor;
}

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float2 crd = float2(DTid.xy);
	float2 dims = float2(Res.xy);
	float2 NrmPos = crd / dims;
	float Roughness = GBUFFER_BaseSpec.SampleLevel(g_Clampsampler, NrmPos, 0).w;
	if (Roughness >= VX_MinRoughness && Roughness <= VX_MaxRoughness)
	{
#if 1
		float3 Pos = GBUFFER_Pos.SampleLevel(g_Clampsampler, NrmPos, 0).xyz;
		float3 Normal = GBUFFER_Normal.SampleLevel(g_Clampsampler, NrmPos, 0).xyz;
#else
		Roughness = 1.0f;
		coneDirection = ViewDir;
#endif
		float3 ViewDir = normalize(CameraPos - Pos);
		uint seed = initRand(DTid.x + DTid.y*Res.x, FrameCount);
		float3 AccumColour = float3(1, 0, 0);
		//	Roughness = 1.0f;
#if 0
		float3 H = getGGXMicrofacet(seed, 1.0 - Roughness, Normal);
		float3 coneDirection = normalize(2.f * dot(ViewDir, H) * H - ViewDir);
#else
		float3 coneDirection = -reflect(ViewDir, Normal);
#endif
		float aperture = tan(Roughness * PI * 0.5f * 0.1f);

		TracePayload payload = (TracePayload)0;
		RayTileData TileData = (RayTileData)0;
		float t = ExecuteConeTrace(VoxelTransferMap, Pos, normalize(Normal), coneDirection, aperture, TileData);
		if (t > 0.0f)
		{
			AccumColour = LightVoxels(TileData);
		}
		else
		{
			AccumColour = SpecularBlurMap.SampleLevel(Textures, coneDirection, (1.0 - Roughness) * (MAX_REFLECTION_LOD)).rgb;
		}
		gOutput[DTid.xy] = float4(AccumColour, 1.0f);
	}
	else
	{
		gOutput[DTid.xy] = float4(0, 0, 0, 0);
	}
}