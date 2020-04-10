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
	uint2 POS = uint2(DTid.x * 8, DTid.y);
	uint index = flatten2D(DTid.xy, Res);
	uint Encode = CompressedData[index];
	uint2 Start = POS.xy;
	for (int i = 0; i < 8; i++)
	{
		float4 Colour = float4(1, 1, 1, 1);
		Start.x = POS.x + i;
		uint Value = Encode >> i;
		Colour.x =  Value & 0b1;
		TargetFrameBuffer[Start] = Colour;
	}
}