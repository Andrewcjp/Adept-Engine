Texture2D Texture : register(t0);
sampler DefaultSampler :register(s0);
cbuffer ConstantBuffer : register(b0)
{
	float4 color;
}

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	return float4(Texture.Sample(DefaultSampler,float2(0.5,0)).xyz,1.0f);
	return float4(1,color.y,0,1);
}