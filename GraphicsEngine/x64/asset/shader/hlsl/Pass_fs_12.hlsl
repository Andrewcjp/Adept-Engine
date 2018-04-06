Texture2D texColour : register(t0);
SamplerState defaultSampler : register (s0);

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};


float4 main(VS_OUTPUT input) : SV_Target
{
	//return float4(1,1,1,1);
	float4 output = texColour.Sample(defaultSampler, input.uv);

	float exposure = 2.5f;
	float4 mapped = float4(1.0,1.0,1.0,1.0) - exp(-output * exposure);

#if 0
	float gamma = 1.0f / 2.2f;
	output = pow(output, float4(gamma, gamma, gamma, gamma));
#endif
	return mapped;
	
}
