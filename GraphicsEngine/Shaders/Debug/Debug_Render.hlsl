SamplerState defaultSampler : register (s0);
SamplerState g_Clampsampler : register(s1);
Texture2D InputTexture : register(t0);

cbuffer t :register(b0)
{
	int VisAplha;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
	if (VisAplha)
	{
		return float4(InputTexture.Sample(defaultSampler, input.uv).a,0.0f,0.0f, 1.0f);
	}
	return float4(InputTexture.Sample(defaultSampler, input.uv).rgb,1.0f);
}
