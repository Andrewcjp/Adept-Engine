Texture2D texColour : register(t0);
Texture2D OthertexColour : register(t1);
SamplerState defaultSampler : register (s0);

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};


float4 main(VS_OUTPUT input) : SV_Target
{
	float3 output = texColour.Sample(defaultSampler, float2(input.uv.x * 2,input.uv.y)).xyz;
	if (input.uv.x > 0.5f)
	{
		output = OthertexColour.Sample(defaultSampler, float2(input.uv.x * 2, input.uv.y)).xyz;
	}
	const float gamma = 1.0f / 2.2f;
	output = pow(abs(output), float3(gamma, gamma, gamma));
	return float4(output,1.0f);
}
