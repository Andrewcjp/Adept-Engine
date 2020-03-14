#include "Utils\BitConvert.hlsl"
#if FULL_UAV_LOAD
#define SAFEUAVLOAD RWTexture2D<float4> 
float4 LoadUAV(uint2 pos,RWTexture2D<float4> tex)
{
	return tex[pos];
}
void WriteUAV(float4 Value, uint2 pos, RWTexture2D<float4> tex)
{
	tex[pos] = Value;
}
#else
#define SAFEUAVLOAD RWTexture2DArray<uint> 
float4 LoadUAV(uint2 pos, RWTexture2DArray<uint> tex)
{
	float2 RG = D3DX_R16G16_FLOAT_to_FLOAT2(tex[uint3(pos, 0)]);
	float2 BA = D3DX_R16G16_FLOAT_to_FLOAT2(tex[uint3(pos, 1)]);
	return float4(RG, BA);
}
void WriteUAV(float4 Value,uint2 pos, RWTexture2DArray<uint> tex)
{
	tex[uint3(pos, 0)] = D3DX_FLOAT2_to_R16G16_FLOAT(float2(Value.r, Value.g));
	tex[uint3(pos, 1)] = D3DX_FLOAT2_to_R16G16_FLOAT(float2(Value.b, Value.a));
}
#endif