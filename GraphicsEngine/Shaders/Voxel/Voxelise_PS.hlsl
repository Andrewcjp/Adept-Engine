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
uint EncodeColour(float3 color)
{
	float hdr = length(color.rgb);
	color.rgb /= hdr;
	uint3 iColor = uint3(color.rgb * 255.0f);
	uint colorMask = (iColor.r << 16u) | (iColor.g << 8u) | iColor.b;
	return colorMask;
}
float3 DecodeColour(uint packed)
{
	float3 color = float3(0, 0, 0);
	color.r = (packed >> 16u) & 0x000000ff;
	color.g = (packed >> 8u) & 0x000000ff;
	color.b = packed & 0x000000ff;
	return color;
}
uint EncodeNormal()
{
	return 0;
}
void SetHit(uint4 pack)
{
	pack.a = 1 << 7;
}
uint4 EncodeSample(float3 Colour, float3 Normal)
{
	uint4 data = uint4(0, 0, 0, 0);

	SetHit(data);
	return data;
}
bool SampleHit(uint4 packed)
{
	return packed.a;
}
Texture2D BaseTex: register(t12);
SamplerState defaultSampler : register(s0);
RWTexture3D<float4> voxelTex;
void main(PSInput input)
{
	float3 diff = (input.P - VoxelGridCenter) * VoxelRes_INV * VoxelSize_INV;
	float3 uvw = diff * float3(0.5f, -0.5f, 0.5f) + 0.5f;

	uint3 writecoord = floor(uvw * VoxelRes);
	float3 samplec = BaseTex.Sample(defaultSampler, input.uv.xy).rgb;
	samplec = max(samplec, voxelTex[writecoord].xyz);
	voxelTex[writecoord] = float4(samplec, 1);
	//voxelTex[writecoord] = float4(input.Normal, 1);
}