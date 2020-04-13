//static const float3 VoxelGridCenter = float3(0, 0, 0);
//static const float VoxelSize = 0.1;
//static const float VoxelSize_INV = 1.0 / VoxelSize;
//static const float VoxelRes = 50;
//static const float VoxelRes_INV = 1.0/ VoxelRes;
static const uint  VoxelConeMaxMIPs = 5;

static const float SQRT2 = 1.41421356237309504880;
static const float VoxelMaxDistance = 250;
static const float __hdrRange = 10.0f;
cbuffer VoxelDataBuffer : register(b66)
{
	float3 VoxelGridCenter;
	float VoxelSize;
	float VoxelSize_INV;
	uint3 VoxelRes;
	float3 VoxelRes_INV;
};
static const float VoxelRayStepDistance = VoxelSize;
uint EncodeColor(in float4 color)
{
	// normalize color to LDR
	float hdr = length(color.rgb);
	color.rgb /= hdr;

	// encode LDR color and HDR range
	uint3 iColor = uint3(color.rgb * 255.0f);
	uint iHDR = (uint)(saturate(hdr / __hdrRange) * 127);
	uint colorMask = (iHDR << 24u) | (iColor.r << 16u) | (iColor.g << 8u) | iColor.b;

	// encode alpha into highest bit
	uint iAlpha = (color.a > 0 ? 1u : 0u);
	colorMask |= iAlpha << 31u;

	return colorMask;
}

// Decode 32 bit uint into HDR color with 1 bit alpha
float4 DecodeColor(in uint colorMask)
{
	float hdr;
	float4 color;

	hdr = (colorMask >> 24u) & 0x0000007f;
	color.r = (colorMask >> 16u) & 0x000000ff;
	color.g = (colorMask >> 8u) & 0x000000ff;
	color.b = colorMask & 0x000000ff;

	hdr /= 127.0f;
	color.rgb /= 255.0f;

	color.rgb *= hdr * __hdrRange;

	color.a = (colorMask >> 31u) & 0x00000001;

	return color;
}
uint EncodeNormal(in float3 normal)
{
	int3 iNormal = int3(normal*255.0f);
	uint3 iNormalSigns;
	iNormalSigns.x = (iNormal.x >> 5) & 0x04000000;
	iNormalSigns.y = (iNormal.y >> 14) & 0x00020000;
	iNormalSigns.z = (iNormal.z >> 23) & 0x00000100;
	iNormal = abs(iNormal);
	uint normalMask = iNormalSigns.x | (iNormal.x << 18) | iNormalSigns.y | (iNormal.y << 9) | iNormalSigns.z | iNormal.z;
	return normalMask;
}

// Decode specified mask into a float3 normal (normalized).
float3 DecodeNormal(in uint normalMask)
{
	int3 iNormal;
	iNormal.x = (normalMask >> 18) & 0x000000ff;
	iNormal.y = (normalMask >> 9) & 0x000000ff;
	iNormal.z = normalMask & 0x000000ff;
	int3 iNormalSigns;
	iNormalSigns.x = (normalMask >> 25) & 0x00000002;
	iNormalSigns.y = (normalMask >> 16) & 0x00000002;
	iNormalSigns.z = (normalMask >> 7) & 0x00000002;
	iNormalSigns = 1 - iNormalSigns;
	float3 normal = float3(iNormal) / 255.0f;
	normal *= iNormalSigns;
	return normal;
}
uint4 EncodeSample(float3 Colour, float3 Normal)
{
	uint4 data = uint4(EncodeColor(float4(Colour,1.0f)), EncodeNormal(Normal), 0, 1);
	//SetHit(data);
	return data;
}
float3 GetPackedNormal(uint4 pack)
{
	return DecodeNormal(pack.y);
}
float3 GetPackedCol(uint4 pack)
{
	return DecodeColor(pack.x).xyz;
}
bool SampleHit(uint4 packed)
{
	return packed.a;
}