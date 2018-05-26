Texture2D texColour : register(t0);
SamplerState defaultSampler : register (s2);

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
	float3 Colour : NORMAL;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	const float SampledValue = texColour.Sample(defaultSampler, input.uv).r;
	float4 sampled = float4(1.0,1.0,1.0, SampledValue);
	return float4(input.Colour, SampledValue);
}
