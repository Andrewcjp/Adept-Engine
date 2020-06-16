RWTexture2D<float> FeedBackMap :register(u0);
RWTexture2D<float> MipMaxMap :register(u1);
Buffer<float> CPU_MipData : register(t0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
	uint w = 0;
	uint h = 0;
	FeedBackMap.GetDimensions(w, h);
	uint Index = DTid.x *w + DTid.y;
	MipMaxMap[DTid.xy] = CPU_MipData[Index];
}