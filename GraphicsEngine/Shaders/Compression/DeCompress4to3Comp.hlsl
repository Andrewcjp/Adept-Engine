RWTexture2D<float4> TargetFrameBuffer : register(u0);
Buffer<uint> CompressedData:  register(t0);
PUSHCONST cbuffer Resolution : register(b0)
{
	int2 Res;
};
cbuffer AreaData : register(b1)
{
	int2 OffsetXY;
};
inline uint flatten2D(uint2 coord, uint2 dim)
{
	return coord.x + (coord.y * dim.x);
}
float UnPackFloat(uint f)
{
	return  f / 255.0f;
}
#if USE_8BIT
#define DECODE(x) UnPackFloat(x);
#else
#define DECODE(x) f16tof32(x);
#endif
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 groupIndex : SV_GroupID)
{
	float4 Colour = float4(1, 1, 1, 1);
	uint index = flatten2D(DTid.xy, Res) * 3;
	Colour.x = DECODE(CompressedData[index]);
	Colour.y = DECODE(CompressedData[index + 1]);
	Colour.z = DECODE(CompressedData[index + 2]);
	TargetFrameBuffer[OffsetXY + DTid.xy] = Colour;
}