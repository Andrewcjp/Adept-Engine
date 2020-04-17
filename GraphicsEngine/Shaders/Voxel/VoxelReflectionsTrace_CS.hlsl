#include "Shading/ReflectionsTrace.hlsl"
#include "../Lighting.hlsl"
Texture3D<uint4> voxelTex :  register(t8);
RWTexture2D<float4> gOutput : register(u0);
Texture2D<float4> GBUFFER_Normal : register(t5);
Texture2D<float4> GBUFFER_Pos : register(t6);
Texture2D<float4> GBUFFER_BaseSpec : register(t7);

Texture3D<float> VoxelTransferMap : register(t9);

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
};
void AddVoxelHit(float3 Pos, uint mip,float3 WorldPos, inout RayTileData TileData)
{
	if (mip == 0)
	{
		TileData.Tiles[TileData.Hits] = Pos;
		TileData.WorldPos[TileData.Hits] = WorldPos;
		TileData.Hits++;
	}
}

inline float ExecuteConeTrace(in Texture3D<float> voxels, in float3 P, in float3 N, in float3 coneDirection, in float coneAperture, inout RayTileData TileData)
{
	float3 color = 0;
	float alpha = 0;
	const float VoxelRadianceDataMIPs = 5;
	// We need to offset the cone start position to avoid sampling its own voxel (self-occlusion):
	//	Unfortunately, it will result in disconnection between nearby surfaces :(
	float dist = VoxelSize; // offset by cone dir so that first sample of all cones are not the same
	float3 startPos = P + N * VoxelSize * 2 * SQRT2; // sqrt2 is diagonal voxel half-extent

	// We will break off the loop if the sampling distance is too far for performance reasons:
	while (dist < VoxelMaxDistance && alpha < 1)
	{
		float diameter = max(VoxelSize, 2 * coneAperture * dist);
		float mip = 0;//log2(diameter * VoxelSize_INV);

		// Because we do the ray-marching in world space, we need to remap into 3d texture space before sampling:
		//	todo: optimization could be doing ray-marching in texture space
		float3 ray = startPos + coneDirection * dist;
		float3 tc = (ray - VoxelGridCenter) * VoxelSize_INV;
		tc *= VoxelRes_INV;
		tc = tc * float3(0.5f, -0.5f, 0.5f) + 0.5f;

		// break if the ray exits the voxel grid, or we sample from the last mip:
		if (!is_saturated(tc) || mip >= (float)VoxelRadianceDataMIPs)
			break;

		float Contsample = voxels.SampleLevel(g_Clampsampler, tc, 0);
		float sam = voxels[tc * VoxelRes];

		if (sam > 0.0f || Contsample > 0.5f)
		{
		//	float3 Pos = tc * VoxelRes;
		//	AddVoxelHit(Pos, mip, TileData);
		//}
		//else if (sam > 0.0f)
		//{
			float3 Pos = tc * VoxelRes;
			AddVoxelHit(Pos, mip, ray, TileData);
			//AddVoxelHit(Pos + float3(1, 1, 1), mip, TileData);
			//AddVoxelHit(Pos - float3(1, 1, 1), mip, TileData);
		/*	if (dist > VoxelSize)
			{
				AddVoxelHit(Pos + float3(0, 1, 0), mip, TileData);
				AddVoxelHit(Pos - float3(0, 1, 0), mip, TileData);
				AddVoxelHit(Pos + float3(1, 0, 0), mip, TileData);
				AddVoxelHit(Pos - float3(1, 0, 0), mip, TileData);
				AddVoxelHit(Pos + float3(0, 0, 1), mip, TileData);
				AddVoxelHit(Pos - float3(0, 0, 1), mip, TileData);

				AddVoxelHit(Pos + float3(0, 1, 1), mip, TileData);
				AddVoxelHit(Pos - float3(0, 1, 1), mip, TileData);
				AddVoxelHit(Pos + float3(1, 1, 0), mip, TileData);
				AddVoxelHit(Pos - float3(1, 1, 0), mip, TileData);
				AddVoxelHit(Pos + float3(0, 1, 1), mip, TileData);
				AddVoxelHit(Pos - float3(0, 1, 1), mip, TileData);
			}*/
#if 0
			//use 3 samples to determine source Voxel pos
			float x = voxels.SampleLevel(g_Clampsampler, tc + float3(VoxelRes_INV.x, 0, 0), 0);
			float y = voxels.SampleLevel(g_Clampsampler, tc + float3(0, -VoxelRes_INV.y, 0), 0);
			float z = voxels.SampleLevel(g_Clampsampler, tc + float3(0, 0, VoxelRes_INV.z), 0);
			float3 SrcPos = tc - float3(x, y, z);
			AddVoxelHit(SrcPos, mip, TileData);
#endif
			return 1.0f;
		}

		// this is the correct blending to avoid black-staircase artifact (ray stepped front-to back, so blend front to back):
		float a = 1 - alpha;
		alpha += a * sam;

		// step along ray:
		dist += diameter * VoxelRayStepDistance;
	}

	return alpha;
}
float3 LightVoxels(in RayTileData TileData)
{
	if (TileData.Hits == 0)
	{
		return float3(0, 0, 0);
	}
	float3 VoxelColour = float3(1,1,1);
	float3 Normal = float3(0, 1, 0);
	float3 WorldPos = float3(0, 0, 0);
	for (int i = 0; i < TileData.Hits; i++)
	{
		uint3 SamplePos = floor(TileData.Tiles[i]);
		uint4 Sample = voxelTex[SamplePos];
		if (Sample.a == 0)
		{
			continue;
		}
		VoxelColour = GetPackedCol(Sample);
		Normal = GetPackedNormal(Sample);
		WorldPos = TileData.WorldPos[i];
		break;
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
		float3 Pos = GBUFFER_Pos.SampleLevel(g_Clampsampler, NrmPos, 0).xyz;
		float3 Normal = GBUFFER_Normal.SampleLevel(g_Clampsampler, NrmPos, 0).xyz;
		float3 ViewDir = normalize(CameraPos - Pos);
		uint seed = initRand(DTid.x + DTid.y*Res.x, FrameCount);
		float3 AccumColour = float3(0, 1, 0);
		Roughness = 1.0f;
		float3 H = getGGXMicrofacet(seed, 1.0 - Roughness, Normal);
		float3 coneDirection = normalize(2.f * dot(ViewDir, H) * H - ViewDir);
		float aperture = tan(Roughness * PI * 0.5f * 0.1f);
		TracePayload payload = (TracePayload)0;
		RayTileData TileData = (RayTileData)0;
		float t = ExecuteConeTrace(VoxelTransferMap, Pos, normalize(Normal), coneDirection, aperture, TileData);
		if (t > 0.0f)
		{
			AccumColour = LightVoxels(TileData);
		}
		gOutput[DTid.xy] = float4(AccumColour, Roughness);
	}
	else
	{
		gOutput[DTid.xy] = float4(0, 0, 0, 0);
	}
}