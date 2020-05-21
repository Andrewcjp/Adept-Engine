SamplerState defaultSampler : register (s0);
SamplerState g_Clampsampler : register(s1);

Texture2D PosTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D AlbedoTexture : register(t2);
Texture2D PerSampledShadow: register(t13);
//#if !VULKAN
//Texture2D g_Shadow_texture[MAX_DIR_SHADOWS]: register(t4, space1);
//TextureCube g_Shadow_texture2[MAX_POINT_SHADOWS] : register(t5, space2);
//#endif
#include "Core/Common.hlsl"
#include "Lighting.hlsl"
#include "Shadow.hlsl"
cbuffer LightBuffer : register(b1)
{
	int LightCount;
	int2 TileCount;
};

StructuredBuffer<Light> LightList : register(t20);
StructuredBuffer<int> LightIndexs : register(t21);
RWTexture2D<float4> OutBuffer: register(u0);
cbuffer SceneConstantBuffer : register(b2)
{
	row_major matrix View;
	row_major matrix Projection;
	float3 CameraPos;
	int2 Resolution;
};

#define MAX_LIGHTS_PER_TILE MAX_LIGHTS
[numthreads(LIGHTCULLING_TILE_SIZE, LIGHTCULLING_TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 DGid : SV_GroupThreadID, uint3 groupID : SV_GroupID, uint  groupIndex : SV_GroupIndex)
{
	float2 UV = (float2)DTid.xy / (float2)Resolution;
	float4 pos = PosTexture[DTid.xy];
	float4 Normalt = NormalTexture[DTid.xy];
	float3 Normal = normalize(Normalt.xyz);
	float4 AlbedoSpec = AlbedoTexture[DTid.xy];
	float Roughness = AlbedoSpec.a;
	float Metallic = Normalt.a;
	float3 ViewDir = normalize(CameraPos - pos.xyz);

	float3 output = float3(0, 0, 0);

	uint TileIndex = flatten2D(groupID.xy, TileCount) * MAX_LIGHTS_PER_TILE;
	int TileLightCount = LightIndexs[TileIndex];
	//[unroll(MAX_LIGHTS)]
	for (int i = 0; i < TileLightCount; i++)
	{
		int LightIndex = LightIndexs[TileIndex + 1 + i];
		if (LightIndex == -1)
		{
			break;
		}
		float3 LightColour = CalcColorFromLight(LightList[LightIndex], AlbedoSpec.xyz, pos.xyz, normalize(Normal.xyz), CameraPos, Roughness, Metallic);
		if (/*LightList[LightIndex].PreSampled.x &&*/ LightList[LightIndex].HasShadow)
		{
			LightColour *= (1.0 - PerSampledShadow[DTid.xy].r/*[LightList[LightIndex].PreSampled.y]*/);
		}
		output += LightColour;
	}
	float4 Current = OutBuffer[DTid.xy];
	OutBuffer[DTid.xy] = float4(Current + output, Current.a);
}
