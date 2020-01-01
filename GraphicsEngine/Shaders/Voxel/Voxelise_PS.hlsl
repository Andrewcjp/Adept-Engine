#include "Voxel/VoxelCommon.hlsl"
struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 Normal : NORMAL;
	float3 P : POSITION3D;
};
inline uint flatten3D(uint3 coord, uint3 dim)
{
	return (coord.z * dim.x * dim.y) + (coord.y * dim.x) + coord.x;
}


void SetHit(uint4 pack)
{
	pack.a = 1 << 7;
}

Texture2D BaseTex: register(t12);
SamplerState defaultSampler : register(s0);
RWTexture3D<uint4> voxelTex;
void main(PSInput input)
{
	float3 diff = (input.P - VoxelGridCenter) * VoxelRes_INV * VoxelSize_INV;
	float3 uvw = diff * float3(0.5f, -0.5f, 0.5f) + 0.5f;

	uint3 writecoord = floor(uvw * VoxelRes);
	float3 samplec = BaseTex.Sample(defaultSampler, input.uv.xy).rgb;
#if 0
	samplec = max(samplec, GetPackedCol(voxelTex[writecoord]));
	float3 SmoothNormal = max(input.Normal, GetPackedNormal(voxelTex[writecoord]));
	voxelTex[writecoord] = EncodeSample(samplec, SmoothNormal);
#else
	if (voxelTex[writecoord].a > 0.0f)
	{
		return;
	}
	voxelTex[writecoord] = EncodeSample(samplec, input.Normal);
#endif
}