Texture2D texColour : register(t0);
SamplerState defaultSampler : register (s2);

cbuffer LineConstantBuffer : register(b0)
{
	row_major matrix projection;
	float3 Colour;
}

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	//return float4(Colour,1.0);
	//float sampled = texColour.Sample(defaultSampler, input.uv).rgba;
	//return texColour.Sample(defaultSampler, input.uv).rgba;// float4(sampled, sampled, sampled, sampled);
	const float SampledValue = texColour.Sample(defaultSampler, input.uv).r;
	float4 sampled = float4(1.0,1.0,1.0, SampledValue);
	return float4(Colour, SampledValue);
}
