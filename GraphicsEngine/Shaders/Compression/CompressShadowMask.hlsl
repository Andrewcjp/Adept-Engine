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
	uint2 POS = uint2(DTid.x * 8, DTid.y);
	uint index = flatten2D(DTid.xy, Res);
	
	uint2 Start = POS.xy;
	uint Encode = 0;
	for (int i = 0; i < 8; i++)
	{
		Start.x = POS.x + i;
		float4 Sample = InputData[Start];
		uint Cast = (uint)Sample.x;
		uint Value = Cast << i;
		Encode |= Value;
	}
	OutputBuffer[index] = Encode;
}