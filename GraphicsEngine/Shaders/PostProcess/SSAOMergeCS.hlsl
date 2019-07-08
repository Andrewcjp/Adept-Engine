Texture2D<float4> SrcTexture : register(t0);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState BilinearClamp : register(s0);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float4 Inputcolour = SrcTexture[DTid.xy];
#if 1
	DstTexture[DTid.xy] = DstTexture[DTid.xy] *(Inputcolour.x);
#else
	DstTexture[DTid.xy] = Inputcolour;
#endif
}
