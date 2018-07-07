Texture2D texColour : register(t0);
SamplerState defaultSampler : register (s0);
Texture2D AdditiveBlendTarget : register(t1);
struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};


float4 main(VS_OUTPUT input) : SV_Target
{
	//return float4(1,1,1,1);
	float3 output = texColour.Sample(defaultSampler, input.uv).rgb;

	output += AdditiveBlendTarget.Sample(defaultSampler, input.uv).rgb;


	float gamma = 1.0f / 2.2f;
	float exposure = 2.5;
	output = float3(1.0, 1.0, 1.0) - exp(-output * exposure);
	output = pow(output, float3(gamma, gamma, gamma));

	return float4(output,1.0);	
}
