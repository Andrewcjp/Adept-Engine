sampler Default : register(s0);
RWTexture2D<float4>  sTexture :register(u0);

#define SIGMA 50.0
#define BSIGMA 0.1
#define MSIZE 4

cbuffer Data :register(b0)
{
	int2 Res;
};

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float3 c = sTexture[DTid.xy].rgb;
	float3 final_colour = float3(0,0,0);
	int kSize = 4;

	for (int i = -kSize; i <= kSize; ++i)
	{
		for (int j = -kSize; j <= kSize; ++j)
		{
			final_colour += sTexture[DTid.xy + int2(i, j)];
		}
	}
	final_colour /= kSize* kSize;
	sTexture[DTid.xy] = float4(final_colour, sTexture[DTid.xy].a);

}