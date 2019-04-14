SamplerState defaultSampler : register (s0);
SamplerState g_Clampsampler : register(s1);
Texture2D Texture : register(t0);

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	return float4(Texture.Sample(defaultSampler, input.uv).rgb,1.0f);
}
