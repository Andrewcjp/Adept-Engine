Texture2D InputData : register(t0);
RWBuffer<uint> OutputBuffer:  register(u0);
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
uint PackFloat(float f)
{
	f *= 255;
	uint value = (uint)f;
	return  value;
}

#if USE_8BIT
#define ENCODE(x) PackFloat(x);
#else
#define ENCODE(x) f32tof16(x);
#endif
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 groupIndex : SV_GroupID)
{
	uint index = flatten2D(DTid.xy, Res)*3;
	float4 Sample = InputData[OffsetXY+DTid.xy];
	OutputBuffer[index] = ENCODE(Sample.x);
	OutputBuffer[index + 1] = ENCODE(Sample.y);
	OutputBuffer[index + 2] = ENCODE(Sample.z);
}